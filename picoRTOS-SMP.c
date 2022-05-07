#include "picoRTOS-SMP.h"

/* CHECK FOR OBVIOUS ERRORS */
#if CONFIG_DEFAULT_STACK_COUNT < ARCH_MIN_STACK_COUNT
# error Default stack is too small
#endif

/* TASKS */

/* to avoid static inline in picoRTOS.h, this is duplicated */
void picoRTOS_task_init(struct picoRTOS_task *task,
                        picoRTOS_task_fn_t fn, void *priv,
                        picoRTOS_stack_t *stack,
                        picoRTOS_size_t stack_count)
{
    arch_assert(stack_count >= (picoRTOS_size_t)ARCH_MIN_STACK_COUNT);

    task->fn = fn;
    task->stack = stack;
    task->stack_count = stack_count;
    task->priv = priv;
}

/* SMP SCHEDULER main structures */

typedef enum {
    PICORTOS_SMP_TASK_STATE_EMPTY,
    PICORTOS_SMP_TASK_STATE_READY,
    PICORTOS_SMP_TASK_STATE_RUNNING,
    PICORTOS_SMP_TASK_STATE_SLEEP
} picoRTOS_SMP_task_state_t;

struct picoRTOS_SMP_task_core {
    /*@temp@*/ picoRTOS_stack_t *sp;
    picoRTOS_SMP_task_state_t state;
    picoRTOS_tick_t tick;
    picoRTOS_mask_t core;
#ifdef CONFIG_CHECK_STACK_INTEGRITY
    /*@temp@*/ picoRTOS_stack_t *stack;
    picoRTOS_size_t stack_count;
#endif
};

struct picoRTOS_SMP_core {
    picoRTOS_size_t index[CONFIG_SMP_CORES];
    volatile picoRTOS_tick_t tick;
    struct picoRTOS_SMP_task_core task[CONFIG_TASK_COUNT];
    struct picoRTOS_SMP_task_core idle[CONFIG_SMP_CORES];
    /* CORES STACKS */
    struct {
        /* core 0 already has a stack, this is a waste of memory
         * but we keep this empty space for simplicity */
        picoRTOS_stack_t stack[ARCH_SMP_MIN_STACK_COUNT];
        picoRTOS_stack_t idle_stack[ARCH_MIN_STACK_COUNT];
    } core[CONFIG_SMP_CORES];
};

/* user tasks + idle */
#define TASK_COUNT (CONFIG_TASK_COUNT + CONFIG_SMP_CORES)

/* main core component */
static struct picoRTOS_SMP_core picoRTOS;

static void idle_tasks_setup(void)
{
    picoRTOS_size_t i;

    for (i = 0; i < (picoRTOS_size_t)CONFIG_SMP_CORES; i++) {
        struct picoRTOS_task idle;

        picoRTOS_task_init(&idle, arch_idle, NULL,
                           picoRTOS.core[i].idle_stack,
                           (picoRTOS_size_t)ARCH_MIN_STACK_COUNT);

        picoRTOS.idle[i].sp = arch_prepare_stack(&idle);
        picoRTOS.idle[i].state = PICORTOS_SMP_TASK_STATE_READY;
        picoRTOS.idle[i].core = (picoRTOS_mask_t)(1u << i);
#ifdef CONFIG_CHECK_STACK_INTEGRITY
        picoRTOS.idle[i].stack = picoRTOS.core[i].idle_stack;
        picoRTOS.idle[i].stack_count = (picoRTOS_size_t)ARCH_MIN_STACK_COUNT;
#endif
    }
}

/* SCHEDULER functions */

void picoRTOS_init(void)
{
    /* zero all tasks with no memset */
    picoRTOS_size_t n = (picoRTOS_size_t)TASK_COUNT;

    while (n-- != 0) {
        picoRTOS.task[n].sp = NULL;
        picoRTOS.task[n].state = PICORTOS_SMP_TASK_STATE_EMPTY;
        picoRTOS.task[n].tick = 0;
        picoRTOS.task[n].core = 0;
#ifdef CONFIG_CHECK_STACK_INTEGRITY
        picoRTOS.task[n].stack = NULL;
        picoRTOS.task[n].stack_count = 0;
#endif
    }

    /* IDLE */
    idle_tasks_setup();

    picoRTOS.tick = 0;
    n = (picoRTOS_size_t)CONFIG_SMP_CORES;

    /* all cores start in idle */
    while (n-- != 0)
        picoRTOS.index[n] = (picoRTOS_size_t)CONFIG_TASK_COUNT + n;
}

void picoRTOS_add_task(struct picoRTOS_task *task,
                       picoRTOS_priority_t prio)
{
    /* check params */
    arch_assert(prio < (picoRTOS_priority_t)CONFIG_TASK_COUNT);
    arch_assert(picoRTOS.task[prio].state == PICORTOS_SMP_TASK_STATE_EMPTY);

    picoRTOS.task[prio].sp = arch_prepare_stack(task);
    picoRTOS.task[prio].state = PICORTOS_SMP_TASK_STATE_READY;
    picoRTOS.task[prio].core = PICORTOS_SMP_CORE_ANY;
#ifdef CONFIG_CHECK_STACK_INTEGRITY
    picoRTOS.task[prio].stack = task->stack;
    picoRTOS.task[prio].stack_count = task->stack_count;
#endif
}

void picoRTOS_SMP_set_core_mask(picoRTOS_priority_t prio,
                                picoRTOS_mask_t core_mask)
{
    arch_assert(prio < (picoRTOS_priority_t)CONFIG_TASK_COUNT);
    arch_assert(picoRTOS.task[prio].state == PICORTOS_SMP_TASK_STATE_READY);

    picoRTOS.task[prio].core = core_mask;
}

void picoRTOS_start(void)
{
    arch_smp_init();

    /* start auxiliary cores first */
    picoRTOS_core_t i;

    for (i = (picoRTOS_core_t)1;
         i < (picoRTOS_core_t)CONFIG_SMP_CORES; i++) {
        /* allocate a master stack & idle */
        arch_core_init(i, picoRTOS.core[i].stack,
                       (picoRTOS_size_t)ARCH_MIN_STACK_COUNT,
                       picoRTOS.idle[i].sp);
    }

    /* start scheduler on core #0 */
    arch_start_first_task(picoRTOS.idle[0].sp);
}

void picoRTOS_suspend()
{
    arch_spin_lock();
    arch_suspend();
}

void picoRTOS_resume()
{
    arch_resume();
    arch_spin_unlock();
}

void picoRTOS_schedule(void)
{
    arch_yield();
}

void picoRTOS_sleep(picoRTOS_tick_t delay)
{
    picoRTOS_size_t index = picoRTOS.index[arch_core()];
    struct picoRTOS_SMP_task_core *task = &picoRTOS.task[index];

    arch_assert(task->state == PICORTOS_SMP_TASK_STATE_RUNNING);

    arch_spin_lock();

    if (delay > 0) {
        task->tick = picoRTOS.tick + delay;
        task->state = PICORTOS_SMP_TASK_STATE_SLEEP;
    }

    arch_memory_barrier();
    arch_spin_unlock();

    arch_yield();
}

void picoRTOS_sleep_until(picoRTOS_tick_t *ref, picoRTOS_tick_t period)
{
    picoRTOS_size_t index = picoRTOS.index[arch_core()];
    struct picoRTOS_SMP_task_core *task = &picoRTOS.task[index];

    arch_assert(period > 0);
    arch_assert(task->state == PICORTOS_SMP_TASK_STATE_RUNNING);

    arch_spin_lock();

    /* compute next wakeup */
    *ref += period;

    task->tick = *ref;
    task->state = PICORTOS_SMP_TASK_STATE_SLEEP;

    arch_memory_barrier();
    arch_spin_unlock();

    arch_yield();
}

void picoRTOS_kill(void)
{
    picoRTOS_size_t index = picoRTOS.index[arch_core()];

    picoRTOS.task[index].state = PICORTOS_SMP_TASK_STATE_EMPTY;
    arch_yield();
}

picoRTOS_priority_t picoRTOS_self(void)
{
    return picoRTOS.index[arch_core()];
}

picoRTOS_stack_t *picoRTOS_switch_context(picoRTOS_stack_t *sp)
{
    picoRTOS_size_t previous_index;
    picoRTOS_core_t core = arch_core();
    picoRTOS_size_t index = picoRTOS.index[core];
    picoRTOS_mask_t mask = (picoRTOS_mask_t)(1u << core);

    arch_assert(index < (picoRTOS_size_t)CONFIG_TASK_COUNT);

#ifdef CONFIG_CHECK_STACK_INTEGRITY
    arch_assert(sp != NULL);
    arch_assert(sp >= picoRTOS.task[index].stack);
    arch_assert(sp < (picoRTOS.task[index].stack +
                      picoRTOS.task[index].stack_count));
#endif

    arch_spin_lock();

    /* store current sp */
    picoRTOS.task[index].sp = sp;

    /* remember index */
    previous_index = index;

    do {
        index++;
        /* ignore tasks ran by other cores or not ready */
        if ((picoRTOS.task[index].core & mask) != 0 &&
            picoRTOS.task[index].state == PICORTOS_SMP_TASK_STATE_READY)
            break;
        /* jump out on idle anyway */
    } while (index < (CONFIG_TASK_COUNT + (picoRTOS_size_t)core));

    picoRTOS.index[core] = index;
    picoRTOS.task[index].state = PICORTOS_SMP_TASK_STATE_RUNNING;

    /* make previous task available for other cores */
    if (picoRTOS.task[previous_index].state == PICORTOS_SMP_TASK_STATE_RUNNING)
        picoRTOS.task[previous_index].state = PICORTOS_SMP_TASK_STATE_READY;

    arch_memory_barrier();
    arch_spin_unlock();

    return picoRTOS.task[index].sp;
}

picoRTOS_stack_t *picoRTOS_tick(picoRTOS_stack_t *sp)
{
    picoRTOS_core_t core = arch_core();
    picoRTOS_size_t index = picoRTOS.index[core];
    picoRTOS_mask_t mask = (picoRTOS_mask_t)(1u << core);

#ifdef CONFIG_CHECK_STACK_INTEGRITY
    arch_assert(sp != NULL);
    arch_assert(sp >= picoRTOS.task[index].stack);
    arch_assert(sp < (picoRTOS.task[index].stack +
                      picoRTOS.task[index].stack_count));
#endif

    arch_spin_lock();

    /* store current sp */
    picoRTOS.task[index].sp = sp;

    /* make task available for everyone */
    picoRTOS.task[index].state = PICORTOS_SMP_TASK_STATE_READY;

    /* advance tick (only on core #0) */
    if (core == 0)
        picoRTOS.tick++;

    /* quick pass on sleeping tasks + idle */
    picoRTOS_size_t n = (picoRTOS_size_t)TASK_COUNT;

    while (n-- != 0) {
        if (picoRTOS.task[n].state == PICORTOS_SMP_TASK_STATE_SLEEP &&
            picoRTOS.task[n].tick == picoRTOS.tick)
            /* task is ready to rumble */
            picoRTOS.task[n].state = PICORTOS_SMP_TASK_STATE_READY;

        /* select highest priority ready task */
        if ((picoRTOS.task[n].core & mask) != 0 &&
            picoRTOS.task[n].state == PICORTOS_SMP_TASK_STATE_READY)
            index = n;
    }

    picoRTOS.index[core] = index;
    picoRTOS.task[index].state = PICORTOS_SMP_TASK_STATE_RUNNING;

    arch_memory_barrier();
    arch_spin_unlock();

    return picoRTOS.task[index].sp;
}

picoRTOS_tick_t picoRTOS_get_tick(void)
{
    return picoRTOS.tick;
}

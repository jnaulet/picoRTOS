#ifndef PICORTOS_PORT_H
#define PICORTOS_PORT_H

/* interfaces required by the e200 series of chips */

/* CHECK FOR ERRORS */
#ifndef CONFIG_ARCH_PPC_E200_INTC
# error Interrupt controller address is not defined !
#endif
#ifndef CONFIG_ARCH_PPC_E200_INTC_IACKR
# error Interrupt controller IACKR address is not defined !
#endif
#ifndef CONFIG_ARCH_PPC_E200_TIMER
# error Timer adddress is not defined !
#endif
#ifndef CONFIG_ARCH_PPC_E200_TIMER_IRQ
# error Tick timer IRQ is not set !
#endif

/* INTC */
extern void arch_intc_init(void);

/* TICK */
extern void arch_timer_init(void);
extern void arch_timer_isr(void);

/* REGISTER ACCESS */
/*@external@*/ extern unsigned long arch_MSR(void);
/*@external@*/ /*@temp@*/ extern unsigned long *arch_IVPR(void);

#endif

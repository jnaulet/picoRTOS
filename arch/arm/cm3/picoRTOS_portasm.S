	.syntax unified
	
	/* Declarations */
	.global arch_start_first_task
	.global arch_SYSTICK
	.global arch_PENDSV

	.global arch_test_and_set
	.global arch_compare_and_swap
	
	.extern picoRTOS_tick
	.extern picoRTOS_switch_context

	.macro SAVE_CONTEXT
	mrs r0, psp			/* use r0 as psp stack */
	stmdb r0!, {r4-r11}		/* save registers r4-r11 */
	msr psp, r0		    	/* store new top of stack */
	.endm

	.macro RESTORE_CONTEXT rS
	ldmia \rS!, {r4-r11}	/* restore registers r4-11 */
	msr psp, \rS		/* reclaim stack space */
	.endm
	
	.align 1
	.thumb_func
	.type arch_start_first_task, %function
arch_start_first_task:
	RESTORE_CONTEXT r0
	/* switch to psp stack */
	movs r0, #2
	msr CONTROL, r0
	isb

	pop {r0-r5}	/* pop the registers that are saved automatically */
	mov lr, r5	/* lr is now in r5 */
	pop {r3}	/* return address is now in r3 */
	pop {r2}	/* pop and discard XPSR */
	cpsie i		/* enable interrupts */
	bx r3		/* finally, jump to the user defined task code */

	.align 1
	.thumb_func
	.type arch_SYSTICK, %function
arch_SYSTICK:
	push {lr}		/* store lr for interrupt return */
	SAVE_CONTEXT

	mrs r0, psp
	bl picoRTOS_tick

	RESTORE_CONTEXT r0
	pop {r3}	/* restore lr */
	bx r3
	
	.align 1
	.thumb_func
	.type arch_PENDSV, %function
arch_PENDSV:
	push {lr}		/* store lr (interrupt return) */
	SAVE_CONTEXT

	mrs r0, psp
	bl picoRTOS_switch_context

	RESTORE_CONTEXT r0
	pop {r3}		/* restore lr */
	bx r3

	.align 1
	.thumb_func
	.type arch_test_and_set, %function
arch_test_and_set:
	dmb ish
	ldrex r2, [r0]
	cmp r2, #0
	bne arch_test_and_set_return
	mov r2, #1
	strex r3, r2, [r0]
	cmp r3, #0
	bne arch_test_and_set_return
	dmb ish
	mov r0, #0
arch_test_and_set_return:
	bx lr

	.align 1
	.thumb_func
	.type arch_compare_and_swap, %function
arch_compare_and_swap:
	dmb ish
	ldrex r3, [r0]
	cmp r3, r1
	bne arch_compare_and_swap_return
	/* TBD */
	strex r3, r2, [r0]
	bne arch_compare_and_swap_return
	dmb ish
	mov r0, r1
arch_compare_and_swap_return:
	bx lr

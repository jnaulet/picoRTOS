	.syntax unified
	
	/* Declarations */
	.global arch_start_first_task
	.global arch_SYSTICK
	.global arch_PENDSV
	
	.extern picoRTOS_tick
	.extern picoRTOS_switch_context

	.macro SAVE_CONTEXT
	mrs r0, psp			/* allocate stack */
	subs r0, r0, #32
	msr psp, r0		    	/* store new top of stack */
	stmia r0!, {r4-r7}		/* store the low registers */
	mov r4, r8			/* store the high registers */
	mov r5, r9
	mov r6, r10
	mov r7, r11
	stmia r0!, {r4-r7}
	.endm

	.macro RESTORE_CONTEXT rS
	adds \rS, \rS, #16	/* load the high registers */
	ldmia \rS!, {r4-r7}
	mov r8, r4
	mov r9, r5
	mov r10, r6
	mov r11, r7
	msr psp, \rS	/* reclaim stack space */
	subs \rS, \rS, #32	/* load the low registers */
	ldmia \rS!, {r4-r7}
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
	pop {r2}	/* pop and discard context state */
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

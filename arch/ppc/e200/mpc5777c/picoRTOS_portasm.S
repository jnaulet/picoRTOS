	.global arch_TICK

	.equ INTC_IACKR, 0xfff48008
	.equ INTC_EOIR, 0xfff48018
	.equ PIT_TFLG3, 0xc3ff013c
	
	#include "../picoRTOS_common.S"

	/* TICK ISR */
	.align 4
	.type arch_TICK, %function
arch_TICK:
	mflr r31            /* save lr in r31 */
	mfsprg r3, 1        /* load task stack pointer from SPRG1 */
	e_bl picoRTOS_tick
	mtsprg 1, r3        /* store returned task stack pointer */
	/* reset timer */
	e_li r7, 1
	e_lis r3, PIT_TFLG3@ha	/* reset timer */
	e_stw r7, PIT_TFLG3@l(r3)
	mtlr r31            /* restore lr */
	se_blr

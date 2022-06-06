	/* picoRTOS */
	.extern picoRTS_switch_context
	.extern picoRTOS_tick

	/* arch-specific */
	.global arch_start_first_task
	.global arch_compare_and_swap

	/* special purpose registers */
	.global arch_MSR
	.global arch_IVPR
	.global arch_R13
	.global arch_core
	.global arch_memory_barrier

	/* exceptions */
	.global IVOR4_Handler
	.global IVOR8_Handler

	/* macros */
	.macro SAVE_CONTEXT
	e_sub16i r1, r1, 0x8c /* allocate stack */
	sthcx. r1, 0, r1      /* turn off reservation bit */
	e_stmvsrrw 0x0 (r1)   /* save SRR[0-1] */
	e_stmvsprw 0x8 (r1)   /* save CR, LR, CTR, XER */
	e_stmvgprw 0x18 (r1)  /* save r[0, 3-12] */
	e_stmw r14, 0x44 (r1) /* save r[14-31] */
	.endm

	.macro RESTORE_CONTEXT
	e_lmvsrrw 0x0 (r1)    /* restore SRR[0-1] */
	e_lmvsprw 0x8 (r1)    /* restore CR, LR, CTR, XER */
	e_lmvgprw 0x18 (r1)   /* restore r[0, 3-12] */
	e_lmw r14, 0x44 (r1)  /* restore r[14-31] */
	e_add16i r1, r1, 0x8c /* re-claim stack space */
	.endm

	.macro ADD_CORE_OFFSET_TO reg
	mfpir r2
	se_slwi r2, 2
	add \reg, r2, \reg
	.endm

	/* ARCH_START_FIRST_TASK */
	.align 4
	.type arch_start_first_task, %function
arch_start_first_task:
	mtsprg 0, r1 /* save kernel stack pointer to SPRG0 */
	se_mr r1, r3 /* first task stack is arch_start_first_task param 1 */
	RESTORE_CONTEXT
	se_rfi
	
	/* ARCH_COMPARE_AND_SWAP */
	.align 4
	.type arch_compare_and_swap, %function
arch_compare_and_swap:
	mbar
	lharx r7, 0, r3
	se_cmp r7, r4
	se_bne .arch_compare_and_swap_return
	sthcx. r5, 0, r3
	se_bne .arch_compare_and_swap_return
	mbar
	se_mr r3, r4	/* success */
.arch_compare_and_swap_return:
	se_blr

	/* ARCH_MSR */
	.align 4
	.type arch_MSR, %function
arch_MSR:
	mfmsr r3
	se_blr

	/* ARCH_IPVR */
	.align 4
	.type arch_IPVR, %function
arch_IVPR:
	mfivpr r3
	se_blr

	/* ARCH_R13 */
	.align 4
	.type arch_R13, %function
arch_R13:
	mr r3, r13
	se_blr

	/* ARCH_CORE */
	.align 4
	.type arch_core, %function
arch_core:
	mfpir r3
	se_blr

	/* MEMORY BARRIER */
	.align 4
	.type arch_memory_barrier, %function
arch_memory_barrier:
	mbar
	se_blr

	.align 4
arch_EE: /* EXTERNAL EXCEPTION */
IVOR4_Handler:
	SAVE_CONTEXT
	mtsprg 1, r1 /* store task stack pointer to SPRG1 */
	mfsprg r1, 0 /* load kernel stack pointer from SPRG0 */

	/* put ISR address in lr */
	e_lis  r3, INTC_IACKR@ha
	ADD_CORE_OFFSET_TO r3
	e_lwz  r3, INTC_IACKR@l(r3)
	se_lwz r3, 0x0(r3)
	se_mtlr r3

	/* put private field in r3 */
	e_rlwinm r4, r3, 0, 20, 30 /* extract intvec */
	e_lis r3, arch_EE_private@ha
	add   r3, r4, r3
	e_lwz r3, arch_EE_private@l(r3)

	/* branch to ISR with return address */
	se_blrl

	/* ack interrupt */
	e_lis r3, INTC_EOIR@ha
	ADD_CORE_OFFSET_TO r3
	e_stw r3, INTC_EOIR@l(r3)
	
	mtsprg 0, r1 /* save kernel stack pointer to SPRG0 */
	mfsprg r1, 1 /* load task stack pointer from SPRG1 */
	RESTORE_CONTEXT
	sync
	se_rfi

	/* SYSCALL */
	.align 4
arch_SC:
IVOR8_Handler:
	SAVE_CONTEXT
	se_mr r3, r1 /* move task stack pointer to r3 */
	mfsprg r1, 0 /* load kernel stack pointer from SPRG0 */
	e_bl picoRTOS_switch_context
	mtsprg 0, r1 /* save kernel stack pointer to SPRG0 */
	se_mr r1, r3 /* switch to newly select task stack */
	RESTORE_CONTEXT
	sync
	se_rfi

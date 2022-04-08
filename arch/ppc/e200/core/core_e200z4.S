	.global arch_core_start
	.extern arch_start_first_task

	/* global variables */
	.global arch_core_sp
	.global arch_task_sp
	.global arch_core_ivpr

	.section .sdata

	.align 4
arch_core_sp:
	.long 0
arch_task_sp:
	.long 0
arch_core_ivpr:
	.long 0
	
	.section .text

	.align 4
	.type arch_core_start, %function
arch_core_start:
core_e200z4_start:
	wrteei 0 /* disable interrupts */
	
	/* enable MSR[ME] */
	e_li r31, 0x1000
	mtmsr r31

	/* set IVPR */
	e_lis r3, arch_core_ivpr@ha
	e_lwz r3, arch_core_ivpr@l (r3)
	mtivpr r3
	
	/* set r1 / main stack */
	e_lis r1, arch_core_sp@ha
	e_lwz r1, arch_core_sp@l (r1)
	
	/* set r3 to task stack */
	e_lis r3, arch_task_sp@ha
	e_lwz r3, arch_task_sp@l (r3)
	
	e_bl arch_start_first_task

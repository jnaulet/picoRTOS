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
core_e200z7_start:
	wrteei 0 /* disable interrupts */
	
	/* enable MSR[ME] */
	e_li r31, 0x1000
	mtmsr r31

	/* set IVPR */
	e_lis r3, arch_core_ivpr@ha
	e_lwz r3, arch_core_ivpr@l (r3)
	mtivpr r3

	/* set IVORs, as they are not set automatically on z7 */
	e_rlwinm r3, r3, 0, 16, 27
	mtivor 400, r3, 0x0    /* IVOR0 400 Critical input */
	mtivor 401, r3, 0x10   /* IVOR1 401 Machine check */
	mtivor 402, r3, 0x20   /* IVOR2 402 Data storage */
	mtivor 403, r3, 0x30   /* IVOR3 403 Instruction storage */
	mtivor 404, r3, 0x40   /* IVOR4 404 External input */
	mtivor 405, r3, 0x50   /* IVOR5 405 Alignment */
	mtivor 406, r3, 0x60   /* IVOR7 406 Program */
	mtivor 407, r3, 0x70   /* IVOR7 407 Floating-point unavailable */
	mtivor 408, r3, 0x80   /* IVOR8 408 System call */
	mtivor 409, r3, 0x90   /* IVOR9 409 Auxiliary processor unavailable */
	mtivor 410, r3, 0xa0   /* IVOR10 410 Decrementer */
	mtivor 411, r3, 0xb0   /* IVOR11 411 Fixed-interval timer interrupt */
	mtivor 412, r3, 0xc0   /* IVOR12 412 Watchdog timer interrupt */
	mtivor 413, r3, 0xd0   /* IVOR13 413 Data TLB error */
	mtivor 414, r3, 0xe0   /* IVOR14 414 Instruction TLB error */
	mtivor 415, r3, 0xf0   /* IVOR15 415 Debug */
	/* IVOR16-IVOR31 - Reserved for future architectural use */
	mtivor 528, r3, 0x100  /* IVOR32 528 SPE APU unavailable */
	mtivor 529, r3, 0x110  /* IVOR33 529 SPE floating-point data exception */
	mtivor 530, r3, 0x120  /* IVOR34 530 SPE floating-point round exception */
	mtivor 531, r3, 0x130  /* IVOR35 531 Performance Monitor Interrupt */
	
	/* set r1 / main stack */
	e_lis r1, arch_core_sp@ha
	e_lwz r1, arch_core_sp@l (r1)
	
	/* set r3 to task stack */
	e_lis r3, arch_task_sp@ha
	e_lwz r3, arch_task_sp@l (r3)
	
	e_bl arch_start_first_task

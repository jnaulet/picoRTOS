	/* Declarations */
	.global arch_start_first_task
	.global arch_TIMER1_COMPA
	.global arch_yield

	.extern picoRTOS_tick
	.extern picoRTOS_switch_context

	.equ SPL, 0x3d
	.equ SPH, 0x3e
	.equ SREG, 0x3f

	.macro SAVE_CONTEXT
	push r0
	in r0, SREG
	push r0
	push r1
	push r2
	push r3
	push r4
	push r5
	push r6
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	push r18
	push r19
	push r20
	push r21
	push r22
	push r23
	push r24
	push r25
	push r26
	push r27
	push r28
	push r29
	push r30
	push r31
	.endm

	.macro RESTORE_CONTEXT
	pop r31
	pop r30
	pop r29
	pop r28
	pop r27
	pop r26
	pop r25
	pop r24
	pop r23
	pop r22
	pop r21
	pop r20
	pop r19
	pop r18
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	pop r6
	pop r5
	pop r4
	pop r3
	pop r2
	pop r1
	pop r0
	out SREG, r0
	pop r0
	.endm

	.type arch_start_first_task, %function
arch_start_first_task:
	/* copy param to SP */
	out SPH, r25
	out SPL, r24
	RESTORE_CONTEXT
	ret

	.type arch_TIMER1_COMPA, %function
arch_TIMER1_COMPA:
	SAVE_CONTEXT

	in r25, SPH
	in r24, SPL
	rcall picoRTOS_tick

	out SPH, r25
	out SPL, r24
	RESTORE_CONTEXT
	reti
	
	.type arch_yield, %function
arch_yield:
	cli	/* mimic interrupt entry behavior */
	SAVE_CONTEXT

	in r25, SPH
	in r24, SPL
	rcall picoRTOS_switch_context

	out SPH, r25
	out SPL, r24
	RESTORE_CONTEXT
	reti	/* automatically re-activate interrupts */

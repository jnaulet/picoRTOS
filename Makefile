CHECKS := c28x cm0+ cm3 e200z4 e200z7 e200z4-smp e200z7-smp avr5 avr6 rp2040
SPLINTFLAGS := -I. -DCONFIG_CHECK_STACK_INTEGRITY -checks -exportlocal

all: $(CHECKS)

c28x: ARCH := arch/c2000/c28x
c28x: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
c28x:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c

cm0+: ARCH := arch/arm/cm0+
cm0+: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
cm0+:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

cm3: ARCH := arch/arm/cm3
cm3: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
cm3:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c \
	  ipc/picoRTOS_mutex.c ipc/picoRTOS_cond.c

e200z4: ARCH := arch/ppc/e200/mpc574xx
e200z4: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z4:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c \
	  ipc/picoRTOS_mutex.c ipc/picoRTOS_cond.c

e200z7: ARCH := arch/ppc/e200/mpc5777c
e200z7: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z7:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c \
	  ipc/picoRTOS_mutex.c ipc/picoRTOS_cond.c

e200z4-smp: ARCH := arch/ppc/e200/mpc574xx
e200z4-smp: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z4-smp:
	splint $(SPLINTFLAGS) picoRTOS-SMP.c $(ARCH)/picoRTOS_port.c \
	  $(ARCH)/picoRTOS-SMP_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c \
	  ipc/picoRTOS_mutex.c ipc/picoRTOS_cond.c

e200z7-smp: ARCH := arch/ppc/e200/mpc5777c
e200z7-smp: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z7-smp:
	splint $(SPLINTFLAGS) picoRTOS-SMP.c $(ARCH)/picoRTOS_port.c \
	  $(ARCH)/picoRTOS-SMP_port.c \
	  ipc/picoRTOS_spinlock.c ipc/picoRTOS_futex.c \
	  ipc/picoRTOS_mutex.c ipc/picoRTOS_cond.c

avr5: ARCH := arch/avr/avr5
avr5: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
avr5:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

avr6: ARCH := arch/avr/avr6
avr6: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
avr6:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

rp2040: ARCH := arch/arm/cm0+/rp2040
rp2040: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
rp2040:
	splint $(SPLINTFLAGS) picoRTOS-SMP.c $(ARCH)/../picoRTOS_port.c \
	  $(ARCH)/picoRTOS-SMP_port.c

linux: ARCH:= arch/pthread/linux
linux: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples +posixlib -unrecog
linux:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

.phony: $(CHECKS)

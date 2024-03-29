CHECKS := c28x cm0+ e200z4 e200z7
# e200z4-smp e200z7-smp
SPLINTFLAGS := -I. -DCONFIG_CHECK_STACK_INTEGRITY -checks -exportlocal

all: $(CHECKS)

c28x: ARCH := arch/c2000/c28x
c28x: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
c28x:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

cm0+: ARCH := arch/arm/cm0+
cm0+: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
cm0+:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c

e200z4: ARCH := arch/ppc/e200
e200z4: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z4:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  $(ARCH)/timer/timer-pit.c $(ARCH)/intc/intc-mpc574xx.c

e200z7: ARCH := arch/ppc/e200
e200z7: SPLINTFLAGS += -I$(ARCH) -I$(ARCH)/samples
e200z7:
	splint $(SPLINTFLAGS) picoRTOS.c $(ARCH)/picoRTOS_port.c \
	  $(ARCH)/timer/timer-pit.c $(ARCH)/intc/intc-mpc5777x.c

.phony: $(CHECKS)

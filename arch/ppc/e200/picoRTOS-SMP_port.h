#ifndef PICORTOS_SMP_PORT_H
#define PICORTOS_SMP_PORT_H

#include "picoRTOS_port.h"

/* SPINLOCK */
extern void arch_spinlock_init(void);

/* CORE */
/*@external@*/ extern void arch_core_start(void);

#endif

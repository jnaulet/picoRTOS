#ifndef PICORTOS_PORT_H
#define PICORTOS_PORT_H

#define INTC_BASE   0xfff48000
#define PIT_BASE    0xc3ff0000
#define PIT_IRQ     304

#define INTC_MCR   ((unsigned long*)INTC_BASE)
#define INTC_CPR   ((unsigned long*)(INTC_BASE + 0x8))
#define INTC_IACKR ((unsigned long*)(INTC_BASE + 0x10))
#define INTC_EOIR  ((unsigned long*)(INTC_BASE + 0x18))
#define INTC_PSR   ((unsigned char*)(INTC_BASE + 0x40))

#define PIT_MCR     ((unsigned long*)PIT_BASE)
/* channel 3 */
#define PIT_LDVAL3  ((unsigned long*)(PIT_BASE + 0x130))
#define PIT_CVAL3   ((unsigned long*)(PIT_BASE + 0x134))
#define PIT_TCTRL3  ((unsigned long*)(PIT_BASE + 0x138))
#define PIT_TFLG3   ((unsigned long*)(PIT_BASE + 0x13c))

#endif

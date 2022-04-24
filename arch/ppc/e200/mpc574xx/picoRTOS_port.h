#ifndef PICORTOS_PORT_H
#define PICORTOS_PORT_H

#define INTC_BASE   0xfc040000
#define PIT_BASE    0xfff84000
#define PIT_IRQ     229

#define INTC_BCR   ((unsigned long*)INTC_BASE)
#define INTC_CPR   ((unsigned long*)(INTC_BASE + 0x10))
#define INTC_IACKR ((unsigned long*)(INTC_BASE + 0x20))
#define INTC_EOIR  ((unsigned long*)(INTC_BASE + 0x30))
#define INTC_PSR   ((unsigned short*)(INTC_BASE + 0x60))

#define PIT_MCR     ((unsigned long*)PIT_BASE)
/* channel 3 */
#define PIT_LDVAL3  ((unsigned long*)(PIT_BASE + 0x130))
#define PIT_CVAL3   ((unsigned long*)(PIT_BASE + 0x134))
#define PIT_TCTRL3  ((unsigned long*)(PIT_BASE + 0x138))
#define PIT_TFLG3   ((unsigned long*)(PIT_BASE + 0x13c))

#endif

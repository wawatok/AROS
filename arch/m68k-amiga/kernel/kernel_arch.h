/*
 * Machine-specific definitions.
 *
 * This file needs to be replaced for every machine. Hosted ports
 * may share the same file in arch/all-$(ARCH)/kernel/kernel_arch.h
 *
 * This file is just a sample providing necessary minimum.
 */

/* Number of IRQs used in the machine. Needed by kernel_base.h */
#define IRQ_COUNT 14

/*
 * Interrupt controller functions. Actually have the following prototypes:
 *
 * void ictl_enable_irq(uint8_t num);
 * void ictl_disable_irq(uint8_t num);
 */

#define ictl_enable_irq(irq)
#define ictl_disable_irq(irq)

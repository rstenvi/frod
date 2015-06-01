/**
* \file pic.c
* Code to handle the 8259 PIC.
*/

#include "sys/kernel.h"
#include "hal/hal.h"
#include "lib/string.h"
#include "lib/stdio.h"

#define PIC_ICW1_ICW4      0x01
#define PIC_ICW1_SINGLE    0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL     0x08
#define PIC_ICW1_INIT      0x10

#define PIC_IVW4_8086       0x01
#define PIC_IVW4_AUTO       0x02
#define PIC_IVW4_BUF_SLAVE  0x08
#define PIC_IVW4_BUF_MASTER 0x0C
#define PIC_IVW4_SFNM       0x10


#define PIC_MASTER_VECTOR_OFFSET 32
#define PIC_SLAVE_VECTOR_OFFSET  (PIC_MASTER_VECTOR_OFFSET+8)


// IRQ number where the slave connects to the master
#define PIC_IRQ_SLAVE_MASTER 2


// Default mask is to mask everything except the IRQ that connects the slave to
// the master
#define PIC_ENABLE_MASK_MASTER 0xFF & ~(1<<PIC_IRQ_SLAVE_MASTER)
#define PIC_ENABLE_MASK_SLAVE  0xFF

void pic_disable();
void pic_remap();


void pic_init()	{
	pic_disable();
	pic_remap();
}


void pic_disable()	{
	// Need to mask all interrupts
	outb(PIC_PORT_MASTER_DATA, PIC_ENABLE_MASK_MASTER);
	outb(PIC_PORT_SLAVE_DATA,  PIC_ENABLE_MASK_SLAVE);
}

void pic_remap()	{
	// Save masks
	uint8_t m1 = inb(PIC_PORT_MASTER_DATA);
	uint8_t m2 = inb(PIC_PORT_SLAVE_DATA);

	
	outb(PIC_PORT_MASTER_CMD, (PIC_ICW1_INIT + PIC_ICW1_ICW4));
	outb(PIC_PORT_SLAVE_CMD,  (PIC_ICW1_INIT + PIC_ICW1_ICW4));


	outb(PIC_PORT_MASTER_DATA, PIC_MASTER_VECTOR_OFFSET);
	outb(PIC_PORT_SLAVE_DATA,  PIC_SLAVE_VECTOR_OFFSET);


	outb(PIC_PORT_MASTER_DATA, 4);
	outb(PIC_PORT_SLAVE_DATA,  2);


	outb(PIC_PORT_MASTER_DATA, PIC_IVW4_8086);
	outb(PIC_PORT_SLAVE_DATA,  PIC_IVW4_8086);

	// Restore saved masks
	outb(PIC_PORT_MASTER_DATA, m1);
	outb(PIC_PORT_SLAVE_DATA,  m2);
}

void pic_enable_irq(int irq)	{
	uint8_t m1 = inb(PIC_PORT_MASTER_DATA);
	uint8_t m2 = inb(PIC_PORT_SLAVE_DATA);

	kprintf(K_LOW_INFO, "\tEnabling: %i\n", irq);

	if(irq < 8)
		outb(PIC_PORT_MASTER_DATA, m1 & ~(1<<irq));
	else	{
		irq -= 8;
		outb(PIC_PORT_SLAVE_DATA,  m2 & ~(1<<irq));
	}
}

void pic_disable_irq(int irq)	{
	uint8_t m1 = inb(PIC_PORT_MASTER_DATA);
	uint8_t m2 = inb(PIC_PORT_SLAVE_DATA);

	kprintf(K_LOW_INFO, "\tEnabling: %i\n", irq);

	if(irq < 8)
		outb(PIC_PORT_MASTER_DATA, m1 | (1<<irq));
	else	{
		irq -= 8;
		outb(PIC_PORT_SLAVE_DATA,  m2 | (1<<irq));
	}
}


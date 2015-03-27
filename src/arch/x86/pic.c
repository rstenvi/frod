/**
* \file pic.c
*/

#include "sys/kernel.h"

#define PIC_PORT_MASTER_CMD  0x20
#define PIC_PORT_MASTER_DATA 0x21
#define PIC_PORT_SLAVE_CMD   0xA0
#define PIC_PORT_SLAVE_DATA  0xA1


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


void pic_init()	{
	pic_disable();
	pic_remap();
}


void pic_disable()	{
	// Need to mask all interrupts
	outb(PIC_PORT_MASTER_DATA, 0xFF);
	outb(PIC_PORT_SLAVE_DATA,  0xFF);
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

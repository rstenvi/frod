/**
* \file ioapic.c
* Handle the I/O APIC controller.
*
* IOAPIC is used to handle interrupts in a multiprocessor system, but can also
* be used to handle interrupts in a uniprocessor system. There is typically one
* of these on a system, unless the computer has multiple I/O subsystems.
*
* The unit consist of:
* 1. A set of interrupt signals
* 2. 24 entries of 64 bits (Interrupt Redirection Table (IRT))
* 3. Programmable registers
* 4. Messaging unit.
*
* The IRT is used to format an interrupt message. The IRT can be used to decide
* things like: sensitivity, which CPU should be interrupted etc. IRT has the
* following meaningful bits:
* - 0-7 -- Interrupt vector
* - 8-10 -- Delivery mode
* - 11 -- Destination mode
* - 12 -- Delivery status
* - 13 -- Interrupt Input Pin Polarity
* - 14 -- Remote IRR 
* - 15 -- Trigger mode
* - 16 -- Interrupt mask
* - 17-55 -- Reserved
* - 56-63 -- Destination field (APIC ID or set of processors)
*
* Some registers are:
* 1. I/O register select (IOREGSEL) -- 
* 2. I/O Window Register (IOWIN) -- 
* 3. Version Register
* 4. ID register -- Physical name of the IOAPIC
* 
*/
#include "sys/kernel.h"
#include "hal/apic.h"
#include "lib/stdio.h"

extern uint32_t apicbase;

#define IOAPIC_DEFAULT_ADDR 0xFEC00000

#define IOAPIC_REGSEL_OFF 0x00
#define IOAPIC_IOWIN_OFF  0x10

#define REGSEL (uint32_t*)(IOAPIC_DEFAULT_ADDR + apicbase + IOAPIC_REGSEL_OFF)
#define IOWIN  (uint32_t*)(IOAPIC_DEFAULT_ADDR + apicbase + IOAPIC_IOWIN_OFF)


#define IOAPIC_ID_OFF        0
#define IOAPIC_VERSION_OFF   1
#define IOAPIC_ARBITR_OFF    2
#define IOAPIC_REDIR_TAB_OFF 0x10




#define IOAPIC_IRT_DELMOD_FIXED     (000b << 8)
#define IOAPIC_IRT_DELMOD_LOW       (001b << 8)
#define IOAPIC_IRT_DELMOD_SMI       (010b << 8)
#define IOAPIC_IRT_DELMOD_RESERVED  (011b << 8)
#define IOAPIC_IRT_DELMOD_NMI       (100b << 8)
#define IOAPIC_IRT_DELMOD_INIT      (101b << 8)
#define IOAPIC_IRT_DELMOD_RESERVED2 (110b << 8)
#define IOAPIC_IRT_DELMOD_EXTINT    (111b << 8)

#define IOAPIC_IRT_DESTMOD_LOG (1 << 11)
#define IOAPIC_IRT_DELIVS_RO   (1 << 12)
#define IOAPIC_IRT_INTPOL_LOW  (1 << 13)
#define IOAPIC_IRT_IRR_RO      (1 << 14)
#define IOAPIC_IRT_TRIG_LEVEL  (1 << 15)
#define IOAPIC_IRT_MASK        (1 << 16)


#define IOAPIC_INT_DISABLED   0x00010000


IO_apic io_apic;

// NOTE: Read and write must be on 32-bit boundaries and it must be 32-bit values.

uint32_t ioapic_read(uint32_t* addr, uint32_t reg);
void ioapic_write(uint32_t* addr, uint32_t reg, uint32_t val);


bool ioapic_install()	{
	/*
	// I don't think it matter if there is a mismatch
	uint8_t id = (uint8_t)(ioapic_read(io_apic.addr, IOAPIC_ID_OFF) >> 24);
	if(id != io_apic.id)	{
		kprintf(K_FATAL, "Found %x, expected %x\n", id, io_apic.id);
	}
	*/


	// Disable all interrupts
	kprintf(K_FATAL, "\tMax interr %i\n", io_apic.max_interr);
	int i;
	for(i = 0; i < io_apic.max_interr; i++)	{
		ioapic_write(
			io_apic.addr,
			(IOAPIC_REDIR_TAB_OFF+(2*i)),
			(IOAPIC_INT_DISABLED | (IRQ0 + i))
		);
		ioapic_write(
			io_apic.addr,
			(IOAPIC_REDIR_TAB_OFF+(2*i)+1),
			0
		);
	}

	return false;
}


void ioapic_enable_irq(int irq, int cpu_id)	{
	ioapic_write(
		io_apic.addr,
		IOAPIC_REDIR_TAB_OFF+(2*irq),
		(IRQ0 + irq)
	);
	ioapic_write(
		io_apic.addr,
		IOAPIC_REDIR_TAB_OFF+(2*irq)+1,
		(cpu_id << 24)
	);
}


uint32_t ioapic_read(uint32_t* addr, uint32_t reg)	{
	uint32_t volatile* a = (uint32_t volatile*)addr;
	a[0] = (reg & 0xFF);	// Only use first 8 bits
	return a[4];
}


void ioapic_write(uint32_t* addr, uint32_t reg, uint32_t val)	{
	uint32_t volatile* a = (uint32_t volatile*)addr;
	a[0] = (reg & 0xFF);
	a[4] = val;
}



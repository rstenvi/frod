/**
* \file lapic.c
* Local APIC for handling CPU-specific interrupts.
* This internal non-IO interrupts.
*/

#include "sys/kernel.h"

#include "hal/isr.h"
#include "hal/apic.h"
#include "hal/hal.h"

#include "lib/stdio.h"

volatile uint32_t* local_apic;
extern IO_apic io_apic;


// This is read only in modern architectures
#define LAPIC_ID (0x0020/4)
#define LAPIC_VERSION (0x0030/4)

// Task priority register
#define LAPIC_TPR (0x0080/4)
#define LAPIC_EOI     (0x00B0/4)


/**
* Spurious interrupt vector register, it has the following format:
* - Bit 0-7: Which interrupt vector is delivered. This means which interrupt
*   vector should be called in the case of a spurious interrupt.
* - Bit 8: Enable(1) or disable(0)
* - Bit 9: Whether focus is enabled(1) or disabled(0)
*/
#define LAPIC_SPUR_INTR      (0x00F0/4)
#define LAPIC_TIMER_DIVIDE_REG   (0x03E0/4)
#define LAPIC_TIMER_INIT_CNT (0x0380/4)


#define LAPIC_INTR_TABLE0   (0x0350/4)
#define LAPIC_INTR_TABLE1   (0x0360/4)
#define LAPIC_INTR_TAB_ERR  (0x0370/4)
#define LAPIC_ERROR_STATUS  (0x0280/4)
#define LAPIC_INTR_CMD_LO   (0x0300/4)
	#define INTR_CMD_INIT      0x00000500
	#define INTR_CMD_STARTUP   0x00000600
	#define INTR_CMD_DELIVER   0x00001000
	#define INTR_CMD_ASSERT    0x00004000
	#define INTR_CMD_DEASSERT  0x00000000
	#define INTR_CMD_LEVEL     0x00008000
	#define INTR_CMD_BCAST     0x00080000
	#define INTR_CMD_BUSY      0x00001000
	#define INTR_CMD_FIXED     0x00000000
#define LAPIC_INTR_CMD_HI   (0x0310/4)

#define LAPIC_LVT_TIMER (0x320/4)
	#define LVT_TIMER_PERIODIC 0x00020000


#define LAPIC_ENABLE 0x00000100



#define LAPIC_MASK 0x00010000


void lapic_new_address(uint32_t addr)	{
	local_apic = (uint32_t*)addr;
}

void lapic_write(int index, uint32_t value);

/**
* Determine the bus frequency, needed to say how often an interrupt should
* occurr.
* \returns Returns number of ticks per second for the current divisor.
* Method:
* - Set the LAPIC count and divisor and enable interrupts.
* - Initialize PIT and let it run for 100ms
* - Read how many ticks have happened to get the ticks / 100ms
* - The initial count can then be used to interrupt the processor at the
* appropriate time.
*/
int lapic_get_bus_freq();




bool lapic_install()	{
	// Not an MP or at least we don't have the address
	if(local_apic == NULL)	return false;


	// Enable the local APIC by setting the enable flag in the spurious interrupt
	// vector register.
	lapic_write(LAPIC_SPUR_INTR, LAPIC_ENABLE | IRQ_SPURIOUS);

	
	// Set up the APIC timer, 4 registers:
	// 1 Divide configuration register
	//   - Determines what to divide by.
	// 2 Initial count
	//   - Value to start counting down from
	// 3 Current count
	//   - Set automatically by initial count (read-only)
	// 4 LVT timer
	//   - Specifies the interrupt vector number and how it should trigger, uses
	//   periodic with reloading the count-down value.

	// DCR
	lapic_write(LAPIC_TIMER_DIVIDE_REG, (3 + 8));

	// LVT
	lapic_write(LAPIC_LVT_TIMER, LVT_TIMER_PERIODIC | IRQ_TIMER);

	// Initial count
	lapic_write(LAPIC_TIMER_INIT_CNT, 10000000);



	// Diable logical interrupt lines
	lapic_write(LAPIC_INTR_TABLE0, LAPIC_MASK);
	lapic_write(LAPIC_INTR_TABLE1, LAPIC_MASK);

	// Map error interrupt
	lapic_write(LAPIC_INTR_TAB_ERR, IRQ_ERROR);


	// Clear error status register, must be done twice
	lapic_write(LAPIC_ERROR_STATUS, 0);
	lapic_write(LAPIC_ERROR_STATUS, 0);


	lapic_write(LAPIC_EOI, 0);

	lapic_write(LAPIC_INTR_CMD_HI, 0);
	lapic_write(LAPIC_INTR_CMD_LO, INTR_CMD_BCAST | INTR_CMD_INIT | INTR_CMD_LEVEL);
	while(local_apic[LAPIC_INTR_CMD_LO] & INTR_CMD_DELIVER);



	// Enables interrupts on the APIC, but not on the processor
	lapic_write(LAPIC_TPR, 0);


	// Store maximum number of interrupts in IOAPIC structure
	uint32_t version = local_apic[LAPIC_VERSION];
	io_apic.max_interr = (uint8_t)((version >> 16) & 0xFF);

	return true;
}

int lapic_cpuid()	{
	// The last 8 bits of the local APIC ID identifies the processor ID.
	if(local_apic)
		return (local_apic[LAPIC_ID] >> 24);
}

uint32_t delay(uint32_t ns)	{
	uint32_t i;
	for(i = 0; i < ns; i++)	{
		asm("nop");
	}
	return i;
}

#define CMOS_PORT 0x70
#define CMOS_RET  0x71

void lapic_start_ap(uint8_t ID, uint32_t addr)	{
	uint32_t id = (uint32_t)ID;

	/*
	* CPU start executing in BIOS, so we must do a jump from that code to our
	* location.
	* We must set CMOS location 0x0F to 10, this indicates a "warm reset"
	* BIOS then does an indirect jump to location pointed to by 0x0467
	*/

	uint16_t* warm_reset_vect = (uint16_t*)0x0467;
	outb(CMOS_PORT,   0x0F);
	outb(CMOS_PORT+1, 0x0A);

//	warm_reset_vect = (uint16_t*)( (0x40<<4) | 0x67);
	warm_reset_vect[0] = 0;
	warm_reset_vect[1] = (uint16_t)(addr >> 4);



	// Algorithm from MP specification B.4

	// TODO: Should be some delays here, but for now it works on emulators

	lapic_write(LAPIC_INTR_CMD_HI, (id << 24));

	lapic_write(LAPIC_INTR_CMD_LO,
		INTR_CMD_INIT | INTR_CMD_LEVEL | INTR_CMD_ASSERT);
	while(local_apic[LAPIC_INTR_CMD_LO] & INTR_CMD_DELIVER);


	lapic_write(LAPIC_INTR_CMD_LO, INTR_CMD_INIT | INTR_CMD_LEVEL);
	while(local_apic[LAPIC_INTR_CMD_LO] & INTR_CMD_DELIVER);

	// Startup twice
	lapic_write(LAPIC_INTR_CMD_HI, (id << 24));
	lapic_write(LAPIC_INTR_CMD_LO, INTR_CMD_STARTUP | (addr>>12));
	while(local_apic[LAPIC_INTR_CMD_LO] & INTR_CMD_DELIVER);

	lapic_write(LAPIC_INTR_CMD_HI, (id << 24));
	lapic_write(LAPIC_INTR_CMD_LO, INTR_CMD_STARTUP | (addr>>12));
	while(local_apic[LAPIC_INTR_CMD_LO] & INTR_CMD_DELIVER);
}



int lapic_get_bus_freq()	{
	

	return 0;
}

void lapic_write(int index, uint32_t value)	{
	local_apic[index] = value;

	// Read ID, this is just to wait until it has finished writing
	local_apic[LAPIC_ID];
}

inline uint32_t lapic_read_version()	{
	return local_apic[LAPIC_VERSION];
}



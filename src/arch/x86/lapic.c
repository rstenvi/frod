/**
* \file lapic.c
* Local APIC for handling CPU-specific interrupts.
* This internal non-IO interrupts.
*/

#include "sys/kernel.h"
#include "hal/isr.h"

// Set in apic.c
volatile uint32_t* local_apic;


// This is read only in modern architectures
#define LAPIC_ID (0x0020/4)

// Task priority register
#define LAPIC_TPR (0x0080/4)


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


#define LAPIC_LVT_TIMER (0x320/4)
	#define LVT_TIMER_PERIODIC 0x00020000


#define LAPIC_ENABLE 0x00000100



#define LAPIC_MASK 0x00010000


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

	// This should point to something that is ignored
	// TODO: Unsure what to do about it
//	lapic_write(LAPIC_SPUR_INTR, LAPIC_ENABLE | 0xFF);


	
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



	// Enables interrupts on the APIC, but not on the processor
	lapic_write(LAPIC_TPR, 0);

	return true;
}

int lapic_cpuid()	{
	// The last 8 bits of the local APIC ID identifies the processor ID.
	if(local_apic)
		return (local_apic[LAPIC_ID] >> 24);
}



int lapic_get_bus_freq()	{
	

	return 0;
}

void lapic_write(int index, uint32_t value)	{
	local_apic[index] = value;

	// Read ID, this is just to wait until it has finished writing
	local_apic[LAPIC_ID];
}




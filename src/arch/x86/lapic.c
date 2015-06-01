/**
* \file lapic.c
* Local APIC for handling CPU-specific interrupts.
* This internal non-IO interrupts.
*
* Local Vector Table (LVT)
* - It has 7 different registers:
*  1. CMCI
*  2. Timer
*  3. Thermal Monitor
*  4. Performance Counter
*  5. LINT0
*  6. LINT1
*  7. Error
* - 32-bit register with the following bit masks:
*  - 0-7 - Interrupt vector number
*  - 8-10 - Delivery mode.
*  - 12 - Delivery status
*  - 15 - Trigger mode
*  - 16 - Whether or not interrupt should b masked
*  - 17-18 - Timer mode, one-shot, periodic or TSC.
*   - Only if Timer register
*
* Interrupt Command Register (ICR)
* - Used by software to send interrupt to other processor (IPI).
* - 2 32-bit registers, the bits that must be filled out are:
*  - 0-7 - Vector number
*  - 8-10 - Delivery mode
*  - 11 - Destination mode
*  - 12 - Delivery status (read-only)
*  - 14 - Level
*  - 15 - Trigger mode
*  - 18-19 - Destination shortchand
*  - 56-63 - Destination ID
*
* Handling interrupts (Intel manual 3A 10.8.2):
* 1. If message is sent the that CPU and is not a system-defined interrupt (like
* INIT, SIPI etc), the LAPIC looks for an open slot in interrupt queue. If slot
* is available, it is placed, otherwise a retry message is sent back.
* 2. LAPIC dispatches interrupts from queue to CPU one at a time and based on
* priority.
* 3. Interrupt handler is executed and ends with writing to the EOI register to
* say that the interrupt has been handled.
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
	#define TIMER_DIVIDE_1   (8 + 3)
	#define TIMER_DIVIDE_2   0
	#define TIMER_DIVIDE_4   1
	#define TIMER_DIVIDE_8   2
	#define TIMER_DIVIDE_16  3
	#define TIMER_DIVIDE_32  (8 + 0)
	#define TIMER_DIVIDE_64  (8 + 1)
	#define TIMER_DIVIDE_128 (8 + 2)

#define LAPIC_TIMER_INIT_CNT (0x0380/4)


#define LAPIC_ERROR_STATUS  (0x0280/4)

//---------- ICR ----------------------

#define LAPIC_ICR_CMD_LO   (0x0300/4)
	#define ICR_DELIVERY_MODE_FIXED (0 << 8)
	#define ICR_DELIVERY_MODE_LOW   (1 << 8)
	#define ICR_DELIVERY_MODE_SMI   (2 << 8)
	#define ICR_DELIVERY_MODE_NMI   (4 << 8)
	#define ICR_DELIVERY_MODE_INIT  (5 << 8)
	#define ICR_DELIVERY_MODE_START (6 << 8)

	#define ICR_DEST_MODE_PHYSICAL (0 << 11)
	#define ICR_DEST_MODE_LOGICAL  (1 << 11)

	#define ICR_DELIVERY_STAT_IDLE (0 << 12)
	#define ICR_DELIVERY_STAT_PEND (1 << 12)

	#define ICR_LEVEL_DEASSERT (0 << 14)
	#define ICR_LEVEL_ASSERT   (1 << 14)

	#define ICR_TRIGGER_MODE_EDGE  (0 << 15)
	#define ICR_TRIGGER_MODE_LEVEL (1 << 15)

	#define ICR_DEST_SHORTHAND_NO         (0 << 18)
	#define ICR_DEST_SHORTHAND_SELF       (1 << 18)
	#define ICR_DEST_SHORTHAND_ALL        (2 << 18)
	#define ICR_DEST_SHORTHAND_ALL_UNSELF (3 << 18)

#define LAPIC_ICR_CMD_HI   (0x0310/4)



//---------------- LVT registers ----------------

#define LAPIC_LVT_TIMER (0x320/4)
	#define LVT_TIMER_ONE_SHOT (0 << 17)
	#define LVT_TIMER_PERIODIC (1 << 17)
	#define LVT_TIMER_TSC      (2 << 17)

// The other LVT registers
#define LAPIC_LVT_CMCI    (0x2F0/4)
#define LAPIC_LVT_THERMAL (0x330/4)
#define LAPIC_LVT_PERFORM (0x340/4)
#define LAPIC_LVT_LINT0   (0x350/4)
#define LAPIC_LVT_LINT1   (0x360/4)
#define LAPIC_LVT_ERROR   (0x370/4)
	// The various delivery mode masks
	#define LVT_DELIVERY_MODE_FIXED  (0 << 8)
	#define LVT_DELIVERY_MODE_SMI    (2 << 8)
	#define LVT_DELIVERY_MODE_NMI    (4 << 8)
	#define LVT_DELIVERY_MODE_EXTINT (7 << 8)
	#define LVT_DELIVERY_MODE_INIT   (5 << 8)


	#define LVT_DELIVERY_STAT_IDLE  (0 << 12)
	#define LVT_DELIVERY_STAT_PEND  (1 << 12)

	// Two different trigger modes
	#define LVT_TRIGGER_MODE_EDGE   (0 << 15)
	#define LVT_TRIGGER_MODE_LEVEL  (1 << 15)

	// Masked or not masked
	#define LVT_MASK_NOT_MASKED  (0 << 16)
	#define LVT_MASK_MASKED      (1 << 16)


#define LAPIC_ENABLE 0x00000100

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
* \todo Implement
*/
int lapic_get_bus_freq();


void lapic_send_eoi()	{
	lapic_write(LAPIC_EOI, 0);
}


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
	lapic_write(LAPIC_TIMER_DIVIDE_REG, TIMER_DIVIDE_1);

	// LVT
	lapic_write(LAPIC_LVT_TIMER, LVT_TIMER_PERIODIC | IRQ_TIMER);

	// Initial count
	lapic_write(LAPIC_TIMER_INIT_CNT, 10000000);



	// Diable logical interrupt lines
	lapic_write(LAPIC_LVT_LINT0, LVT_MASK_MASKED);
	lapic_write(LAPIC_LVT_LINT1, LVT_MASK_MASKED);

	// Map error interrupt
	lapic_write(LAPIC_LVT_ERROR, IRQ_ERROR);


	// Clear error status register, must be done twice
	lapic_write(LAPIC_ERROR_STATUS, 0);
	lapic_write(LAPIC_ERROR_STATUS, 0);


	lapic_write(LAPIC_EOI, 0);

	lapic_write(LAPIC_ICR_CMD_HI, 0);	// Broadcast

	lapic_write(LAPIC_ICR_CMD_LO,
		ICR_DEST_SHORTHAND_ALL |
		ICR_DELIVERY_MODE_INIT |
		ICR_TRIGGER_MODE_LEVEL
	);
	while(local_apic[LAPIC_ICR_CMD_LO] & ICR_DELIVERY_STAT_PEND);


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
	return 0;
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

	warm_reset_vect[0] = 0;
	warm_reset_vect[1] = (uint16_t)(addr >> 4);



	// Algorithm from MP specification B.4

	// TODO: Should be some delays here, but for now it works on emulators

	lapic_write(LAPIC_ICR_CMD_HI, (id << 24));

	lapic_write(LAPIC_ICR_CMD_LO,
		ICR_DELIVERY_MODE_INIT |
		ICR_TRIGGER_MODE_LEVEL |
		ICR_LEVEL_ASSERT
	);
	while(local_apic[LAPIC_ICR_CMD_LO] & ICR_DELIVERY_STAT_PEND);


	lapic_write(LAPIC_ICR_CMD_LO,
		ICR_DELIVERY_MODE_INIT |
		ICR_TRIGGER_MODE_LEVEL
	);
	while(local_apic[LAPIC_ICR_CMD_LO] & ICR_DELIVERY_STAT_PEND);


	// Startup twice
	lapic_write(LAPIC_ICR_CMD_HI, (id << 24));
	lapic_write(LAPIC_ICR_CMD_LO,
		ICR_DELIVERY_MODE_START |
		(addr>>12)
	);
	while(local_apic[LAPIC_ICR_CMD_LO] & ICR_DELIVERY_STAT_PEND);


	lapic_write(LAPIC_ICR_CMD_HI, (id << 24));
	lapic_write(LAPIC_ICR_CMD_LO,
		ICR_DELIVERY_MODE_START |
		(addr>>12)
	);
	while(local_apic[LAPIC_ICR_CMD_LO] & ICR_DELIVERY_STAT_PEND);
}



int lapic_get_bus_freq()	{
	// http://forum.osdev.org/viewtopic.php?t=10686

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



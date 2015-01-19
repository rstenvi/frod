/**
* \pit.c
* Uses the Programmable Interval Timer to set up a clock with interrupts.
*/

#include "sys/kernel.h"
#include "hal/isr.h"
#include "hal/hal.h"

#include "lib/stdio.h"

#define PIT_DATA_CHANNEL0	0x40
#define PIT_DATA_CHANNEL1	0x41
#define PIT_DATA_CHANNEL2	0x42
#define PIT_CMD_PORT		0x43

// 0x36 = 
// 00:  Channel 0
// 11:  Low byte and high byte
// 011: Square Wave generator
// 0:   Binary mode
#define PIT_VAL_REPEATER	0x36


//------------------ Global variables ----------------------
volatile uint32_t ticks = 0;


//--------------- Internal function definitions ------------------
uint32_t increment_tick(Registers* regs);



//------------- Public API implementation ---------------------
bool pit_install(uint32_t freq)	{
	// Will be a number to large for 16 bits
	if(freq < 19)	return false;

	register_interrupt_handler(IRQ0, increment_tick);

	uint32_t div = 1193180 / freq;

	// Say that we are going to input the frequency
	outb(PIT_CMD_PORT, PIT_VAL_REPEATER);

	// Lower byte
	outb(PIT_DATA_CHANNEL0, (uint8_t)(div & 0xFF));

	// Higher byte
	outb(PIT_DATA_CHANNEL0, (uint8_t)( (div>>8) & 0xFF ));
	return true;
}

void kwait(uint32_t nticks)	{
	uint32_t end = ticks + nticks;
	while(ticks < end);
}


//-------------- Internal function implementations ---------------------

uint32_t increment_tick(Registers* regs)	{
	ticks++;
	if(ticks % 32 == 0)
		return switch_task(regs);
	return (uint32_t)regs;
}



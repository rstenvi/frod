/**
* \file isr.c
*/

#include "sys/kernel.h"
#include "lib/stdio.h"
#include "hal/isr.h"
#include "hal/hal.h"


//-------------- Global variables -------------------------

isr_t interrupt_handlers[256];




//------------- Internal function definitions -------------------

uint32_t isr_handler(Registers* regs);
uint32_t irq_handler(Registers* regs);

/**
* Dummy callback-function to avoid NULL-pointers. When an interrupt has not been
* defined yet, this is the function that is called.
* \remark At the moment, it does nothing, but this can change in the future.
* \param[in] regs All the registers that where saved.
*/
uint32_t dummy_int_handler(Registers* regs);
uint32_t irq_spurious(Registers* regs);




//-------------- Public function implementation ------------------
void isr_install()	{
	int i;
	for(i = 0; i < 256; i++)	{
		interrupt_handlers[i] = &dummy_int_handler;
	}

	interrupt_handlers[IRQ_SPURIOUS] = &irq_spurious;
}


void register_interrupt_handler(uint8_t n, isr_t handler)	{
	interrupt_handlers[n] = handler;
}




//--------------- Internal function implementation ------------------
uint32_t isr_handler(Registers* regs)	{
	uint8_t int_no = regs->int_no & 0xFF;
	if(interrupt_handlers[int_no] != 0)	{
		isr_t handler = interrupt_handlers[int_no];
		return handler(regs);
	}
	else	{
		// All undefined should go to dummy int
		PANIC("Unhandled\n");
	}
	return 0;	// Will never happen
}


uint32_t irq_handler(Registers* regs)	{
	if (regs->int_no >= 40)	{
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
	if (interrupt_handlers[regs->int_no] != 0)	{
		isr_t handler = interrupt_handlers[regs->int_no];
		return handler(regs);
	}
	return (uint32_t)regs;
}

uint32_t dummy_int_handler(Registers* regs)	{
	(void)regs;
	print_regs(regs, K_BOCHS_OUT);
	kprintf(K_BOCHS_OUT, "int_no %i\n", regs->int_no);
	PANIC("Unhandled exception");
	return 0;
}

uint32_t irq_spurious(Registers* regs)	{
	(void)regs;
	return 0;
}

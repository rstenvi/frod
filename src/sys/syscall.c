


#include "sys/kernel.h"
#include "hal/isr.h"
#include "hal/hal.h"


uint32_t syscall_handler(Registers* regs);


void syscall_init()	{
	register_interrupt_handler(0x80, syscall_handler);
}


uint32_t syscall_handler(Registers* regs)	{
	(void)regs;
	kprintf(K_HIGH_INFO, "SYSCALL\n");

	return 0;
}

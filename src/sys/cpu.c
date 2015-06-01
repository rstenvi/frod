/**
* \cpu.c
* Code for starting the remaining CPUs
*/

#include "sys/kernel.h"
#include "sys/pmm.h"
#include "hal/hal.h"
#include "drv/uart.h"

#include "lib/stdio.h"

#include "sizes.h"


extern cpu_info cpus[];
extern int num_cpus;


void cpu_ap_enter();
void cpu_common_main();

void cpu_start_aps()	{
	int i;
	uint32_t* code = (uint32_t*)0x7000;
	uint32_t curr_stack = KERNEL_STACK_TOP;
	for(i = 0; i < num_cpus; i++)	{
		if(cpus[i].boot_cpu == true)	continue;

		curr_stack -= (KB4*2);
		code[1] = (uint32_t)curr_stack;
		code[2] = (uint32_t)cpu_ap_enter;
		uint32_t pdir = vmm_return_kernel_dir();
		code[3] = pdir;
	
		lapic_start_ap(cpus[i].id, (uint32_t)code); 

		while(cpus[i].started != 1);
	}
	cpu_common_main();
}


void cpu_ap_enter()	{
	gdt_install();
	lapic_install();
	cpu_common_main();
}


void cpu_common_main()	{
	idt_enable();
	xchg(&cpu->started, 1);
	kprintf(K_HIGH_INFO, "[INFO] Reached main with CPU %i\n", cpus[lapic_cpuid()].id);

	enable_int();


	int i = 0, ret;
	for(i = 0; i < 5; i++)	{
		if( (ret = uart_putc('A'+i)) != UART_SUCCESS)	{
			kprintf(K_LOW_INFO, "Failed to print %c | %i\n", ('A'+i), ret);
		}
	}



	uint32_t* r = (uint32_t*)pmm_alloc_first();
	vmm_map_page((uint32_t)r, GB1, X86_PAGE_USER | X86_PAGE_WRITABLE);
	uint16_t* virt = (uint16_t*)GB1;
	virt[0] = 0x80cd;
	virt[1] = 0x80cd;
	virt[2] = 0xfeeb;
	task_enter_usermode();

	// TODO: Rest of the kernel
	while(1);
}



void cpu_print_info(cpu_info* c)	{
	kprintf(K_LOW_INFO, "\tID: %i | started: %i | CLI: %i\n",
		c->id, c->started, c->num_cli);
}

void cpu_print_all()	{
	int i;
	for(i = 0; i < num_cpus; i++)	{
		cpu_print_info(&cpus[i]);
	}
}

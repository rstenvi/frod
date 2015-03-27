/**
* \cpu.c
* Code for starting the remaining CPUs
*/

#include "sys/kernel.h"
#include "sys/pmm.h"
#include "hal/hal.h"

#include "lib/stdio.h"

#include "sizes.h"


extern cpu_info cpus[];
extern int num_cpus;


void cpu_ap_enter();
void cpu_common_main();

void cpu_start_aps()	{
	int i;
	uint8_t* stack;
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

	// TODO: Rest of the kernel
	while(1);
}


void cpu_print_all()	{
	int i;
	for(i = 0; i < num_cpus; i++)	{
		cpu_print_info(&cpus[i]);
	}
}

void cpu_print_info(cpu_info* c)	{
	kprintf(K_LOW_INFO, "\tID: %i | started: %i | CLI: %i\n",
		c->id, c->started, c->num_cli);
}


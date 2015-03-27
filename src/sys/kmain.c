/**
* \file kmain.c
* Entry point in the kernel.
*/

#include "sys/kernel.h"

#include "hal/hal.h"

#include "sys/multiboot1.h"
#include "sys/pit.h"
#include "sys/pmm.h"
#include "sys/vmm.h"
#include "sys/process.h"
#include "sys/heap.h"

#include "drv/vga.h"

#include "lib/stdio.h"


void kmain(multiboot_info *mboot_ptr, uint32_t mem_start, uint32_t mem_end, uint32_t stack)	{
	if(mem_end > MAX_KERNEL_MEM)	{
		PANIC("Kernel is too large\n");
	}
	if(VM_VALID == false)	{
		PANIC("Virtual memory is NOT valid\n");
	}

	// Initialize the VGA screen
	vga_init(DEFAULT_FG, DEFAULT_BG);
	kprintf(K_HIGH_INFO, "[INIT] Screen\n");

//	print_mb_info(mboot_ptr);

	// Check if we multiboot passes us enough information about our kernel
	// This also works as simple sanity check on the structure.
	check_necessary_flags(mboot_ptr->flags);


	// 64-bit is even less supported
	#if defined(x86_64)
		printf("Running 64-bit kernel\n");
		for(;;);
	#endif

#ifdef TEST_KERNEL
	if(pmm_run_all_tests() == false)	{
		kprintf(K_LOW_INFO, "pmm_run_all_tests() failed\n");
		for(;;);
	}
#endif
	
	// Initialize the physical memory manager
	init_pmm((multiboot_mmap*)mboot_ptr->mmap_addr, mboot_ptr->mmap_length);
	kprintf(K_HIGH_INFO, "[INIT] Physical Memory Manager\n");

	// Mark this code as taken
	pmm_mark_mem_taken(mem_start, mem_end);
	
	// Move code that handles bootstrapping of APs into the correct location
	if(!move_module(mboot_ptr, "bootap.bin", 0x7000))	{
		PANIC("Unable to move bootapp.bin");
	}
	
	if(cpu_supported() == 0)
		PANIC("CPU not supported");

#ifdef TEST_KERNEL
	if(string_run_all_tests() == false)	{
		kprintf(K_LOW_INFO, "string_run_all_tests\n");
		for(;;);
	}
	if(kernel_run_all_tests() == false)	{
		for(;;);
	}
#endif

	if(apic_init() == false)	{
		PANIC("APIC NOT found\n");
	}

	int n = apic_find_cpus();
	if(n == 0)	{
		PANIC("0 CPUs\n");
	}
	kprintf(K_LOW_INFO, "[INFO] Found %i CPUs\n", n);

	if(lapic_install() == false)	{
		PANIC("Unable to install local APIC\n");
	}
	kprintf(K_HIGH_INFO, "[INIT] APIC found %i CPUs\n", n);


	uint32_t ver = lapic_read_version();
	kprintf(K_LOW_INFO, "[INFO] LAPIC version: %i, max LVT %i\n",
		ver&0xFF, (ver & 0x00FF0000) >> 16);


	// Install the GDT
	gdt_install();
	kprintf(K_HIGH_INFO, "[INIT] GDT initialized\n");

	// Disable the PIC
	pic_init();
	kprintf(K_HIGH_INFO, "[INIT] PIC initialized\n");

	ioapic_install();
	kprintf(K_HIGH_INFO, "[INIT] IOAPIC initialized\n");

	idt_install();
	kprintf(K_HIGH_INFO, "[INIT] IDT initialized\n");
	
	isr_install();
	kprintf(K_HIGH_INFO, "[INIT] ISR initialized\n");
	
	vmm_initialize();
	kprintf(K_HIGH_INFO, "[INIT] Paging\n");
	
	move_stack(KERNEL_STACK_TOP, KERNEL_STACK_SZ, stack);
	kprintf(K_LOW_INFO, "[INFO] Moved stack to VM 0x%x\n", KERNEL_STACK_TOP);
	
	heap_init();
	kprintf(K_HIGH_INFO, "[INIT] Kernel Heap\n");

	kprintf(K_HIGH_INFO, "[INFO] Starting APs\n");
	cpu_start_aps();
	
	// REMARK: Should never return here
	while(1);

/*
#ifdef TEST_KERNEL
	if(vmm_run_all_tests() == false)	while(true);
#endif


#ifdef TEST_KERNEL
	kprintf(K_HIGH_INFO, "Passed all tests\n");
	for(;;);
#endif
*/
	
	// Not ready yet
//	process_init();


	// TODO: Not ready for interrupts yet
//	enable_int();

}



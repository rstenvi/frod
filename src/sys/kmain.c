/**
* \file kmain.c
* Entry point in the kernel.
*/

#include "sys/kernel.h"

#include "hal/hal.h"

#include "sys/multiboot.h"
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

	// Mark this code as taken
	pmm_mark_mem_taken(mem_start, mem_end);
	
	kprintf(K_HIGH_INFO, "[INIT] Physical Memory Manager\n");

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

	if(lapic_install() == false)	{
		PANIC("Unable to install local APIC\n");
	}
	
	kprintf(K_HIGH_INFO, "[INIT] APIC, found %i CPUs\n", n);

	// Check if we multiboot passes us enough information about our kernel
	// This also works as simple sanity check on the structure.
	if(!check_necessary_flags(mboot_ptr->flags))	while(true);


#ifdef TEST_KERNEL
	if(vmm_run_all_tests() == false)	while(true);
#endif


#ifdef TEST_KERNEL
	kprintf(K_HIGH_INFO, "Passed all tests\n");
	for(;;);
#endif

	// Install the GDT, IDT and ISR
	gdt_install();
	idt_install();
	isr_install();
	kprintf(K_HIGH_INFO, "[INIT] GDT, IDT and ISR\n");
	
	// Once we enable paging, some multiboot values will be lost, only those
	// pages marked as in use by pmm_bitmap is identity mapped.
	vmm_initialize();
	kprintf(K_HIGH_INFO, "[INIT] Paging\n");

	// We move the stack to a more predictable location
	move_stack(KERNEL_STACK_TOP, KERNEL_STACK_SZ, stack);

	heap_init();
	kprintf(K_HIGH_INFO, "[INIT] Kernel Heap\n");

	// Not ready yet
//	process_init();
	
	pit_install(20);

	// TODO: Not ready for interrupts yet
//	enable_int();


	printf("[SUCCESS] All parts initialized\n");
}

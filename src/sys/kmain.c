/**
* \file kmain.c
* Entry point in the kernel.
*/

#include "sys/kernel.h"
#include "sys/vmm.h"

#include "hal/hal.h"
#include "hal/apic.h"

#include "sys/multiboot1.h"
#include "sys/pit.h"
#include "sys/pmm.h"
#include "sys/vmm.h"
#include "sys/process.h"
#include "sys/heap.h"

#include "drv/vga.h"
#include "drv/ps2.h"

#include "lib/stdio.h"


acpi_info ainfo;

void test_main(multiboot_info *mboot_ptr, uint32_t mem_start, uint32_t mem_end, uint32_t stack);




void kmain(multiboot_info *mboot_ptr, uint32_t mem_start, uint32_t mem_end, uint32_t stack)	{
	if(mem_end > MAX_KERNEL_MEM)	{
		PANIC("Kernel is too large\n");
	}
	if(VM_VALID == false)	{
		PANIC("Virtual memory is NOT valid\n");
	}



#ifdef TEST_KERNEL
	test_main(mboot_ptr, mem_start, mem_end, stack);
#endif




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

	
	// Initialize the physical memory manager
	init_pmm((multiboot_mmap*)mboot_ptr->mmap_addr, mboot_ptr->mmap_length);
	kprintf(K_HIGH_INFO, "[INIT] Physical Memory Manager\n");

	// Mark this code as taken
	pmm_mark_mem_taken(mem_start, mem_end);
	
	// Move code that handles bootstrapping of APs into the correct location
	if(!move_module(mboot_ptr, "sc2.bin", (uint8_t*)MODULE1_LOCATION))	{
		PANIC("Unable to move sc2.bin");
	}
	kbd_init(MODULE1_LOCATION);

	if(!move_module(mboot_ptr, "bootap.bin", (uint8_t*)0x7000))	{
		PANIC("Unable to move bootapp.bin");
	}
	
	if(cpu_supported() == 0)
		PANIC("CPU not supported");


	if(apic_init() == false)	{
		PANIC("APIC NOT found\n");
	}

	if(apic_find_info(&ainfo) != 0)	{
		PANIC("Unable to store information about system\n");
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

//	kprintf(K_HIGH_INFO, "%x\n", ainfo.boot_flags);
//	if(ainfo.boot_flags & FADT_BOOT_FLAG_8042)	{
	int16_t res = ps2_init();
	if(res < 0)
		kprintf(K_HIGH_INFO, "[INFO] No PS/2 controller found: %i\n", res);
	else	{
		if(res > 0)
			kprintf(K_HIGH_INFO, "[INFO] 1 PS/2 device found\n");
		else
			kprintf(K_HIGH_INFO, "[INFO] 2 PS/2 devices found\n");
	}
	ps2kbd_init();

	int nn;
	if( (nn = uart_init()) != 0)	{
		kprintf(K_HIGH_INFO, "UART NOT initialized: %i\n", nn);
	}

	syscall_init();
	kprintf(K_HIGH_INFO, "[INIT] Syscalls\n");
	
	vmm_init();
	kprintf(K_HIGH_INFO, "[INIT] Paging\n");
	
	move_stack(KERNEL_STACK_TOP, KERNEL_STACK_SZ, stack);
	kprintf(K_LOW_INFO, "[INFO] Moved stack to VM 0x%x\n", KERNEL_STACK_TOP);
	
	heap_init();
	kprintf(K_HIGH_INFO, "[INIT] Kernel Heap\n");

	nn = process_init();
	kprintf(K_HIGH_INFO, "[INIT] Configured kernel process: %i\n", nn);

//	process_start_usermode();
//	kprintf(K_HIGH_INFO, "[INIT] Configured user mode process\n");

	kprintf(K_HIGH_INFO, "[INFO] Starting APs\n");
	cpu_start_aps();
	
	// REMARK: Should never return here
	while(1);
}


#ifdef TEST_KERNEL
void test_main(multiboot_info *mboot_ptr, uint32_t mem_start, uint32_t mem_end,
	uint32_t stack)	{
	
	vga_init(DEFAULT_FG, DEFAULT_BG);
	int n;
	if( (n = uart_init()) != 0)	{
		kprintf(K_HIGH_INFO, "UART NOT initialized: %i\n", n);
		PANIC("NO UART");
	}

	check_necessary_flags(mboot_ptr->flags);


	if(pmm_run_all_tests_before() == false)
		PANIC("pmm_run_all_tests_before() failed");
	init_pmm((multiboot_mmap*)mboot_ptr->mmap_addr, mboot_ptr->mmap_length);



	if(string_run_all_tests() == false)
		PANIC("string_run_all_tests failed");


	if(kernel_run_all_tests() == false)
		PANIC("kernel_run_all_tests failed");
}
#endif

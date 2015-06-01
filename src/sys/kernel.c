/**
* \file kernel.c
* Various functions and defintions that are common to several parts of the
* kernel.
*/


#include "sys/kernel.h"
#include "sys/pmm.h"
#include "hal/hal.h"

#include "lib/stdio.h"	// kprintf
#include "lib/string.h"	// memcpy

void panic(const char* msg, const char* file, uint32_t line)	{
	clear_int();
	kprintf(K_FATAL, "PANIC: %s (%s:%i)\n", msg, file, line);
	while(1);
}


void print_regs(Registers* regs, uint32_t lev)	{
	kprintf(lev, "ES: 0x%x | DS: 0x%x | EDI: 0x%x\n", regs->es, regs->ds, regs->edi);
	kprintf(lev, "GS: 0x%x | FS: 0x%x\n", regs->gs, regs->fs);
	kprintf(lev, "ESI: 0x%x | EBP: 0x%x | t_esp: 0x%x\n", regs->esi, regs->ebp, regs->tampered_esp);
	kprintf(lev, "EBX: 0x%x | EDX: 0x%x | ECX: 0x%x\n", regs->ebx, regs->edx, regs->ecx);
	kprintf(lev, "EAX: 0x%x | int_no: 0x%x | err_code: 0x%x\n", regs->eax, regs->int_no, regs->err_code);
	kprintf(lev, "EIP: 0x%x | CS: 0x%x | EFLAGS: 0x%x\n", regs->eip, regs->cs, regs->eflags);

}


inline uint32_t xchg(volatile uint32_t* addr, uint32_t n_val)   {
	uint32_t ret;
	asm volatile("lock xchgl %0, %1" :
		"+m" (*addr), "=a" (ret) :
		"1" (n_val) :
		"cc");
	return ret;
}


void move_stack(uint32_t new_stack, uint32_t sz, uint32_t init_esp)	{
	uint32_t i;
	for(i = new_stack; i >= (new_stack - sz); i -= 4096)	{
		uint32_t phys = (uint32_t)pmm_alloc_first();
		vmm_map_page(phys, i, X86_PAGE_WRITABLE);
	}
	uint32_t old_stack;
	get_esp(old_stack);
	
	uint32_t old_base;
	get_ebp(old_base);

	uint32_t offset = new_stack - init_esp;

	uint32_t new_stack_ptr = old_stack + offset;
	uint32_t new_base_ptr = old_base + offset;

	memcpy((uint8_t*)new_stack_ptr, (uint8_t*)old_stack, init_esp-old_stack);

	// Now we change values to fit with the new stack
	for(i = new_stack; i >= (new_stack - sz); i -= 4)	{
		uint32_t tmp = *(uint32_t*)i;
		if( (old_stack < tmp) && (tmp < init_esp) )	{
			tmp += offset;
			uint32_t* tmp2 = (uint32_t*)i;
			*tmp2 = tmp;
		}
	}
	set_esp(new_stack_ptr);
	set_ebp(new_base_ptr);
}


// The main body for calculating checksum.
#define checksum_x(sz)\
int i;\
	sz ret = 0;\
	for(i = 0; i < len; i++)	{\
		ret += data[i];\
	}\
	return ret;

uint8_t checksum_8(uint8_t* data, int len)	{
	checksum_x(uint8_t)
}
uint16_t checksum_16(uint16_t* data, int len)	{
	checksum_x(uint16_t)
}
uint32_t checksum_32(uint32_t* data, int len)	{
	checksum_x(uint32_t)
}
uint64_t checksum_64(uint64_t* data, int len)	{
	checksum_x(uint64_t)
}

void* find_signature(void* data, uint64_t len, int bytes, uint64_t sig, int align)	{
	uint64_t i;
	for(i = 0; i < len; i += align)	{
		if(bytes == 1)	{
			if(*((uint8_t*)(data + i)) == (uint8_t)sig)	return (data + i);
		}
		else if(bytes == 2)	{
			if(*((uint16_t*)(data + i)) == (uint16_t)sig)	return (data + i);
		}
		else if(bytes == 4)	{
			if(*((uint32_t*)(data + i)) == (uint32_t)sig)	return (data + i);
		}
		else if(bytes == 8)	{
			if(*((uint64_t*)(data + i)) == (uint64_t)sig)	return (data + i);
		}
		
	}
	return NULL;
}




#ifdef TEST_KERNEL

int kernel_test_checksum()	{
	#define CHECKSUM_DSZ 10
	uint8_t data[CHECKSUM_DSZ];
	int i;
	for(i = 0; i < CHECKSUM_DSZ; i++)	{
		data[i] = i;
	}
	uint8_t res = checksum_8(data, CHECKSUM_DSZ);
	if(res != (CHECKSUM_DSZ*(CHECKSUM_DSZ-1))/2)		return 1;
	
	for(i = 0; i < CHECKSUM_DSZ; i++)	{
		data[i] = (0xFF-i);
	}

	// Result is manually calculated
	res = checksum_8(data, CHECKSUM_DSZ);
	if(res != 0xC9)	return 2;

	return 0;
}



bool kernel_generic_unit_test(unit_test* tests, const char* func)	{
	int res = 0, count = 0;
	do	{
		if( (res = (*tests[count])()) != 0)	{
			kprintf(K_LOW_INFO, "%s: INDEX: %i FAILED: %i\n", func, count, res);
			return false;
		}
		count++;
	} while(tests[count] != NULL);

	return true;
}




bool kernel_run_all_tests()	{
	unit_test tests[2] = {
		kernel_test_checksum,
		NULL
	};

	int res = 0, count = 0;
	do	{
		if( (res = (*tests[count])()) != 0)	{
			kprintf(K_LOW_INFO, "kernel_run_all_tests(): index: %i FAILED: %i\n", count, res);
			return false;
		}
		count++;
	} while(tests[count] != NULL);

	return true;
}

#endif



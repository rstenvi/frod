/**
* \file kernel.h
* Definitions and functions that should be available throughout the kernel and
* for drivers. Anything that is common for many parts and does not belog in the
* hardware abstraction layer belongs here.
* \todo Move this one level out, makes more sense
*/
#ifndef __KERNEL_H
#define __KERNEL_H


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../hal/isr.h"
#include "../config.h"
#include "../vmlayout.h"
#include "../sizes.h"


#ifdef TEST_KERNEL
	#include "../tests.h"
#endif


/**
* Unrecoverable error, should print a message along with the C file and line
* number. Just a wrapper to panic, to avoid typing the extra macros.
*/
#define PANIC(msg) panic(msg, __FILE__, __LINE__);

inline uint32_t xchg(volatile uint32_t* addr, uint32_t n_val);

/**
* Unrecoverable error, print message and halt the computer.
* \remark Do not call this directly, use PANIC macro.
*/
void panic(const char* msg, const char* file, uint32_t line);



void print_regs(Registers* regs, uint32_t lev);


/**
* Calcuate the checksum of some data.
* \param[in] data The data that we should calculate from.
* \param[in] len Number of bytes we should use from data
* \param[in] bytes Number of bytes in each calulation, this can either be 1, 2,
* 4 or 8. Making data uint8_t*, uint16_t*, uint32_t* or uint64_t*.
*/



uint8_t checksum_8(uint8_t* data, int len);
uint16_t checksum_16(uint16_t* data, int len);
uint32_t checksum_32(uint32_t* data, int len);
uint64_t checksum_64(uint64_t* data, int len);


#ifdef TEST_KERNEL

/**
* Code that is common for (all) main test functions.
*/
#define common_test_code(str,t)\
int res = 0, count = 0;\
do	{\
	if( (res = (*t[count])()) != 0)	{\
		kprintf(K_LOW_INFO, "%s: index %i, FAILED: %i\n", str, count, res);\
		return false;\
	}\
	count++;\
} while(t[count] != NULL);\
return true;

#endif	// TEST_KERNEL



#endif

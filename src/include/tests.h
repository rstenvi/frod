/**
* \file tests.h
* Header file for all the main tests.
* \todo
* - Document what has and hasn't been tested
*/

#ifndef __TESTS_H
#define __TESTS_H


#ifdef TEST_KERNEL

// Where information about test output should go. The levels in stdio.h are
// used.
#define TEST_OUTPUT K_LOW_INFO


// The format we should strive for in each test.
typedef int (*unit_test)();



// -------------- Main tests ------------------------------ 


/**
* Run tests for the string library for the kernel. Defined in string.c.
* This only depends on the stack, so it can pretty much be alled anytime.
* \return Returns true if successfull and false if we failed.
*/
bool string_run_all_tests();


/**
* \todo Does NOT follow the correct format, fix it.
*/
bool heap_run_all_tests();


/**
* \todo Implement
*/
bool vmm_run_all_tests();

/**
* Run all available tests to check the correctness of our physical memory map
* implementation.
* This function assumes that the first block (4 KB) of physical memory is
* available and it will write to that block.
* The first block is used to simulate allocations and deallocations of a memory
* map.
* \return Return true if passed, false if failed
* \remark This function should be called before any other memory initialization
* code is called.
*/
bool pmm_run_all_tests();


/**
* Run tests on some of the global kernel functions (kernel.c).
* \return Return true if passed, false if failed
*/
bool kernel_run_all_tests();


#endif	// TEST_KERNEL

#endif

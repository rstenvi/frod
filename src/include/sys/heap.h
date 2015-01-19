/**
* \ingroup heap
* \file heap.h
* Implementation details:
* - Heap starts at virtual address HEAP_START and grows towards higher
* addresses.
*  - 256 MG of virtual memory is reserved to the kernel heap, but physical
*  blocks are allocated as they are needed.
* - Heap is initialized with one frame of memory (4 KB)
*   - Must call heap_init on startup
* - LLMalloc is a doubly linked list containing all the blocks, both available
* and used.
* - To allocate data, use heap_malloc
*   - This tries to find the FIRST available block with the given size.
*   - If no space exist, it will result in PANIC
* - Magic values
*   - 3B is used for magic values, this is only to prevent accidents, not
*   tampering.

* \test Simple correct usage test cases are performed:
* - Simple alloc at correct address
* - Address is available to write to
* - Allocate over more than 1 4 KB block
* - Dealloc and same realloc gives back correct address
*  - Also checks possible off-by-one error

* \todo
* - Need to be able to allocate blocks larger than 4 KB.
*  - Must also ensure that any size can be allocated, need a function to
*  allocate arbitrary sizes.
* - Handle possible alignment issues
* - Describe new algo
* - New tests:
*  - Free both and allocate 1 larger than the first, check that it's now one
*  block
*/

/**
* \addtogroup heap
* @{
*/

#ifndef __HEAP_H
#define __HEAP_H

#include "kernel.h"
#include "lock.h"

/**
* Structure used to manage one block of virtual memory, used or unused.
* Is purposefullt set to 16B, so that it i easier to align on 4 B boundaries and
* maybe better speed.
* \ẗodo used should be 16 bits and drop magic2
*/
typedef struct	_LLMalloc {
	struct _LLMalloc* next,	/**< Next block */
		* prev;	/**< Previous block */

	/** The size in bytes of this blocks. */
	uint32_t size;

	/** 0 if unused, 1 if used. */
	uint8_t used;

	/** First magic value, set to 0xabcd. */
	uint16_t magic1;

	/** Second magic value, set to 0xef. */
	uint8_t magic2;
} __attribute__ ((packed)) LLMalloc;


/**
* Data about the heap.
*/
typedef struct	{
	spinlock lock;
	LLMalloc* kern_heap;
	uint32_t blocks_allocked;
} Heap;


/**
* Simple wrapper to heap_get_new_block with NULL as parameter.
*/
void heap_init();

/**
* Allocate and return some memory.
* \param[in] sz Number of bytes to allocate.
*/	
void* heap_malloc(uint32_t sz);

/**
* Free a block of memory previously allocated.
* \param[in,out] addr The address to memory, that should be freed, the pointer
* is also set to NULL if we where successfull.
* \remark This is not built with security in mind
*/
void heap_free(void* addr);



#endif


/** @} */	// heap

/**
* \file lock.h
* Header file for the locking mechanism (spinlock).
*/

#ifndef __LOCK_H
#define __LOCK_H

#include "kernel.h"
#include "../hal/hal.h"


/**
* All the different type of shared resources that a CPU can ask for. These are
* ALL the locks the kernel use. They are ordered and trying to acquire a lock
* with a lower id (higher up) than a lock it currently has will result in
* failure. This is used to avoid deadlocks.
*
* Informal rules for order:
* - Resources that is very unlikely to ask for more should be placed at the
* bottom, some examples are: screen / console, heap.
* - Resources close to system calls should be placed at the top, some examples
* are: process lock, virtual fil system.
*/
typedef enum	{
	LOCK_PROC,
	LOCK_VFS,
	LOCK_ATA,
	LOCK_CONSOLE,
	LOCK_HEAP,
	UNKNOWN
} lock_resource;


/**
* 
*/
typedef struct	{
	/**
	* ID is used to determine what kind of resource the lock is controlling. This
	* can be used for debugging purposes.
	*/
	lock_resource ID;
	uint32_t locked;
	uint32_t call_stack;

	/** Which CPU is currently holding the lock. */
	cpu_info* cpu;
} spinlock;



/**
* Initiate a spinlock to NOT taken.
* Each module that is responsible for some shared resource should initiate this
* with an identifier.
* \param[in] lock The structure representing the lock.
* \param[in] id Identifier as to what kind of lock this is. This is used to
* determine if CPU is allowed to acquire the lock or not, it depends on what
* other type of locks it has acquired.
*/
void init_spinlock(spinlock* lock, lock_resource id);

/**
* Acquire the lock.
* \todo Interface is not really defined yet.
*/
void spinlock_acquire(spinlock* lock);


/**
* Release the lock.
*/
void spinlock_release(spinlock* lock);

#endif

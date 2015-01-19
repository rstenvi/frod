/**
* \file lock.c
* Implementation of the spinlock.
* Inspired from xv6
*/

#include "sys/kernel.h"
#include "sys/lock.h"
#include "hal/hal.h"

void pushcli();
void popcli();

void init_spinlock(spinlock* lock, lock_resource id)	{
	lock->locked = 0;
	lock->ID = id;
	lock->cpu = NULL;
}

inline uint32_t alock(volatile uint32_t* addr, uint32_t n_val)	{
	uint32_t ret;
	asm volatile("lock xchgl %0, %1" :
		"+m" (*addr), "=a" (ret) :
		"1" (n_val) :
		"cc");
	return ret;
}

void spinlock_acquire(spinlock* lock)	{
	pushcli();
	// Should check if we are already holding it, as done in xv6

	// Busy waiting until it is available
	// TODO: This is also the time to check how many times we have tried and give
	// up after X number of times. This can be configured in config.h
	while(alock(&lock->locked, 1) != 0);

	// TODO: Set the other variables that are useful to have
}

void spinlock_release(spinlock* lock)	{
	// Should check if we are already holding it
	

	alock(&lock->locked, 0);
	popcli();
}



void pushcli()	{

}


void popcli()	{

}



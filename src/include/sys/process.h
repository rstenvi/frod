/**
* \file process.h
* Header file for implementing multiprogramming.
*
* Data about a process:
* - ID
*  - Unique ID to identify each process.
* - Paging table
*  - Virtual memory space
* - Security token
*  - A token determining what the process has access to.
*  - Several can link to the same, but change will create new
* - State
*  - \todo Determine possible states
*
* All the processes are contained in a linked list.
* \todo Should split this up more, both according to state (can be an array) and
* priority for those waiting to execute (can also be an array).
*
* Creation of processes follows the UNIX/Linux way of forking and copying on
* write.
*/

#ifndef __PROCESS_H
#define __PROCESS_H

#include "kernel.h"
#include "vmm.h"
#include "dllist.h"

#include "../hal/isr.h"


#define PROC_RUNNING 1
#define PROC_READY   2
#define PROC_BLOCKED 3

#define PROCESS_STATES 10

#define KSTACKSZ KB4


typedef struct	{
	uint16_t type;
	uint32_t vm_start;
	uint32_t vm_end;

	uint16_t acl;

} vm_area;


typedef struct	{
	dllist_head* mem_regions;

} memory_desc;


typedef struct	{
	/** How many references to this token. */
	uint32_t references;

	uint32_t real_uid;
	uint32_t effective_uid;
} security_token;

typedef struct	{
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} context;


typedef struct _pcb	{
	uint32_t pid;
	uint32_t* dirtable;

	uint8_t* kstack;
	context* cont;
	Registers* regs;

	uint8_t state;

	struct _pcb* next;

} pcb;


/**
* Holds all information about all the processes on the system.
*/
typedef struct _processes	{
	/**
	* List of all processes on the system
	*/
	dllist_head* proc_lists[PROCESS_STATES];


	/**
	* Indicates the next PID we can use
	*/
	uint32_t biggest_pid;

	/**
	* Pointer to a region of memory where each bit says whether or not a PID is
	* available or not. 1 4KB block will specify PID 0 -> 32.896. This is not
	* used or allocated until we have reached the highest PID.
	*/
	uint32_t* pid_mask_available;

} processes;


/**
* Initialize the first process
*/
int process_init();

#endif

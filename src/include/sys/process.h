/**
* \file process.h
* Header file for implementing multiprogramming.
*/

#ifndef __PROCESS_H
#define __PROCESS_H

#include "kernel.h"
#include "vmm.h"

#include "../hal/isr.h"


#define PROC_RUNNING 1
#define PROC_READY   2
#define PROC_BLOCKED 3

#define KSTACKSZ 4096

typedef struct	{
	uint32_t edi;
	uint32_t esi;
	uint32_t ebx;
	uint32_t ebp;
	uint32_t eip;
} context;


typedef struct _pcb	{
	uint32_t pid;
	vmm_dirtable* dirtable;

	uint8_t* kstack;
	context* cont;
	Registers* regs;

	uint8_t state;

	struct _pcb* next;

} pcb;



/**
* Initialize the first process (the kernel). 
*/
int process_init(/*Registers regs*/);

#endif

/**
* \file acpi.c
* Code for finding and configuring the APIC.
*/


#include "sys/kernel.h"
#include "hal/apic.h"
#include "hal/hal.h"

#include "lib/string.h"		// strncmp
#include "lib/stdio.h"

rsdp_descriptor* rsdp = NULL;

cpu_info cpus[MAX_CPUS];
int num_cpus = 0;
extern uint32_t* local_apic;

uint8_t ioapic_id = NULL;

/**
* Finds out where the Root System Description Pointer (RDSP) is located.
* It can either be in the first 1KB of the EBDA or in the main read-only BIOS area. It is
* aligned on a 16B boundary and the signature is: "RSD PTR "
* \returns Returns the address or 0 on failure. If it didn't fail, it can be
* read using rsdp_descriptor.
*/
uint32_t find_rsdp();
uint32_t find_sdt_entry(rsdp_descriptor* rsdp, uint32_t sig);

bool apic_init()	{
	rsdp = (rsdp_descriptor*)find_rsdp();

	if(rsdp == NULL)	return false;

	// Checksum must also match, otherwise it is not valid.
	// TODO: The first could be a false positive, so maybe we should check the
	// next, if we find it. Just need to:
	// - Do thid in a loop and find_rsdp() must be able to ignore the first N
	// matches.
	if(checksum_8(rsdp, sizeof(rsdp_descriptor)) != 0)
		return false;

	return true;

}


uint32_t find_rsdp()	{
	uint16_t* bda = (uint16_t*)BIOS_DATA_ADDR;
	
	// Address to ebda
	uint64_t* start = (uint8_t*)(bda[7] << 4);

	int i;
	for(i = 0; i < 1024; i += 1)	{
		if(start[i] == RSDP_SIGNATURE)	{
			return (uint32_t)i;
		}
	}

	// If we get here it was not in the ebda
	// We must search the main BIOS area

	for(start = (uint8_t*)MAIN_BIOS_START; start < MAIN_BIOS_END; start += 2)
	{
		if(*start == RSDP_SIGNATURE)	{
			return (uint32_t)start;
		}

	}

	// We didn't find it
	return 0;
}


uint32_t find_sdt_entry(rsdp_descriptor* rsdp, uint32_t sig)	{
	uint64_t pointer = 0;
	uint32_t ptr_sz = 4;
	if(rsdp->revision > 0)	ptr_sz = 8;
	sdth* rsdt = (sdth*)rsdp->rsdt_addr;
	sdth* tmp;
	uint32_t ptr_start = (uint32_t)((uint32_t)rsdt + sizeof(sdth));
	while((uint32_t)ptr_start < ((uint32_t)rsdt + rsdt->length))	{
		if(ptr_sz == 4)
			pointer = *((uint32_t*)ptr_start);
		else
			pointer = *((uint64_t*)ptr_start);
		tmp = (sdth*)pointer;
		if(tmp->sig == sig)	return pointer;
		ptr_start += ptr_sz;
	}
	return 0;

}


// Guidelines for initializing the processors:
// - Initialize them in the order they appear in the MADT.
// Then logical processors are defined the same as physical processors
// Return number of processors
int apic_find_cpus()	{
	madt* m = (madt*)find_sdt_entry(rsdp, APIC_SIGNATURE);
	if(m == NULL)	return 0;

	int ret = 0;

	local_apic = (uint32_t*)m->lapic_addr;

	// TODO: Store lapic addr
	madt_h* tmp = (madt_h*)((uint8_t*)m + sizeof(madt));
	while((uint8_t*)tmp < ((uint8_t*)m + m->header.length))	{
		
		if(tmp->type == MADT_TYPE_LAPIC)	{
			plapic* p = (plapic*)tmp;
			
			cpus[ret].id = p->apic_id;
			// The first listed is the boot cpu
			cpus[ret].boot_cpu = ((ret == 0) ? true : false);
			cpus[ret].started  = ((ret == 0) ? true : false);

			cpus[ret].num_cli = 0;
			cpus[ret].int_enabled = false;

			ret++;
		}
		else if(tmp->type == MADT_TYPE_IOAPIC)	{
			ioapic* io = (ioapic*)tmp;
			ioapic_id = io->ioapic_id;
		}
		tmp = (uint8_t*)tmp + tmp->length;
	}

	num_cpus = ret;
	return ret;
}



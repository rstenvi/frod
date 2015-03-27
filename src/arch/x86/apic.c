/**
* \file acpi.c
* Code for finding and configuring the APIC.
*/


#include "sys/kernel.h"
#include "hal/apic.h"
#include "hal/hal.h"

#include "lib/string.h"		// strncmp
#include "lib/stdio.h"

rsdp_descriptor rsdp;

cpu_info cpus[MAX_CPUS];
int num_cpus = 0;
extern volatile uint32_t* local_apic;

extern IO_apic io_apic;


#define APIC_BASE_MSR 0x1B


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
	int matches = 0;
	uint8_t cs;
	rsdp_descriptor* trsdp = NULL;
	while(true)	{
		trsdp = (rsdp_descriptor*)find_rsdp(matches);

		// No more matches and APIC is not present
		if(trsdp == NULL)	return false;

		// If checksum is not a match, we look for the next one
		if( (cs = checksum_8(trsdp, 20)) != 0)	{
			matches++;
		}
		else	{
			break;
		}
	}

	// If we get here, we have found a match

	// Store for later
	memcpy(&rsdp, trsdp, sizeof(rsdp_descriptor));
	return true;
}

uint32_t find_rsdp(int ignore)	{
	int found = 0;
	uint16_t* bda = (uint16_t*)BIOS_DATA_ADDR;
	
	uint8_t ebda = (uint8_t*)(bda[7] << 4);
	// Address to ebda
	uint8_t* start = ebda;

	uint64_t* cmp;

	int i;
	for(i = 0; i < 1024; i += 1)	{
		cmp = &start[i];
		if(*cmp == RSDP_SIGNATURE)	{
			if(found == ignore)
				return (uint32_t)start + (uint32_t)i;
			else
				found++;
		}
	}

	// If we get here it was not in the ebda
	// We must search the main BIOS area
	start = (uint8_t*)MAIN_BIOS_START;
	for(i = 0; i < (MAIN_BIOS_END-(uint32_t)start); i++)	{
		cmp = &start[i];
		if(*cmp == RSDP_SIGNATURE)	{
			if(found == ignore)
				return (uint32_t)start + (uint32_t)i;
			else
				found++;
		}
	}
	// We didn't find it
	return 0;
}

uint32_t find_sdt_entry(rsdp_descriptor* rsdp, uint32_t sig)	{
	uint64_t pointer = 0;

	sdth* rsdt = (sdth*)rsdp->rsdt_addr;
	sdth* tmp;

	uint32_t ptr_start = (uint32_t)((uint32_t)rsdt + sizeof(sdth));
	
	while((uint32_t)ptr_start < ((uint32_t)rsdt + rsdt->length))	{
		tmp = *((uint32_t*)ptr_start);
		if(tmp->sig == sig)	return (uint32_t)tmp;
		ptr_start += 4;
	}
	return 0;

}



// Guidelines for initializing the processors:
// - Initialize them in the order they appear in the MADT.
// Then logical processors are defined the same as physical processors
// Return number of processors
int apic_find_cpus()	{
	
	madt* m = (madt*)find_sdt_entry(&rsdp, APIC_SIGNATURE);
	if(m == NULL)	return 0;
	local_apic = (uint32_t*)m->lapic_addr;

	int ret = 0;
	#define FOUND_LAPIC  1
	#define FOUND_IOAPIC 2
	uint32_t valid_mask = FOUND_LAPIC | FOUND_IOAPIC;
	uint32_t found_flags = 0;

	// The remaining is one big table of all the necessary data
	madt_h* tmp = (madt_h*)((uint8_t*)m + sizeof(madt));
	while((uint8_t*)tmp < ((uint8_t*)m + m->header.length))	{
		
		if(tmp->type == MADT_TYPE_LAPIC)	{
			plapic* p = (plapic*)tmp;
			
			cpus[ret].id = p->apic_id;
			// The first listed is the boot cpu
			cpus[ret].boot_cpu = ((ret == 0) ? true : false);
			cpus[ret].started  = ((ret == 0) ? 1 : 0);

			cpus[ret].num_cli = 0;
			cpus[ret].int_enabled = false;
			found_flags |= FOUND_LAPIC;
			ret++;
		}
		else if(tmp->type == MADT_TYPE_IOAPIC)	{
			ioapic* io = (ioapic*)tmp;
			io_apic.id = io->ioapic_id;
			io_apic.addr = io->ioapic_addr;
			kprintf(K_LOW_INFO, "\tIO APIC @0x%x, id = %i Base 0x%x\n",
				io_apic.addr, io_apic.id, io->global_sys_intr);
			found_flags |= FOUND_IOAPIC;
		}
		tmp = (uint8_t*)tmp + tmp->length;
	}
	
	num_cpus = ret;
	if((found_flags & valid_mask) != valid_mask)	return 0;
	return ret;
}



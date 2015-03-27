/**
* \file apic.h
*/

#ifndef __APIC_H
#define __APIC_H

#define RSDP_SIGNATURE 0x2052545020445352
#define RSDT_SIGNATURE 0x54445352	// "RSDT"
#define FACS_SIGNATURE 0x53434146	// "FACS"

#define FADT_SIGNATURE 0x50434146	// "FACP"

// Multple APIC Description Table (MADT)
#define APIC_SIGNATURE 0x43495041	// "APIC"
#define DSDT_SIGNATURE 0x54445344	// "DSDT"
#define SSDT_SIGNATURE 0x54445353	// "SSDT"
#define PSDT_SIGNATURE 0x54445350	// "PSDT"
#define SBST_SIGNATURE 0x54534253	// "SBST"
#define DBGP_SIGNATURE 0x50474244	// "DBGP"
// TODO: FIll in the rest
#define XSDT_SIGNATURE 0x00000000 	// "TDSX"
#define BOOT_SIGNATURE 0x00000000	// "TOOB"
#define SRAT_SIGNATURE 0x00000000	// "TARS"
#define WDRT_SIGNATURE 0x00000000	// "TRDW"


#define MADT_TYPE_LAPIC  0
#define MADT_TYPE_IOAPIC 1



/**
* RSDP description for ACPI 1.0. It has since been extended, but that will not
* be implemented until it is needed.
* \todo
* - The is also an extended field if revision is higher than 0
*/
typedef struct	{
	/** Must be RSDP_SIGNATURE */
	char sig[8];

	/** Used to check correctness. */
	uint8_t checksum;

	/** Identifier for the OEM */
	char oemid[6];

	/**
	* Is 0, 1 or 2.
	* - 0 = ACPI 1.0
	* - 1 = ACPI 2.0
	* - 2 = ACPI > 2.0
	*/
	uint8_t revision;

	/**
	* The physical address of the RSDT table.
	*/
	uint32_t rsdt_addr;

	uint32_t length;
} __attribute__((packed)) rsdp_descriptor;


/**
* System Description Table (SDT) header.
* \todo Is this just the header?
*/
typedef struct	{
	/** Determines what kind of payload follows after this header (*_SIGNATURE) */
	uint32_t sig;

	/** The length of this header + the actual payload. */
	uint32_t length;

	
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	char oem_t_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_rev;
} __attribute__((packed))  sdth;


/**
* Root System Descriptor Table (RSDT).
*/
typedef struct	{
	sdth header;

	/**
	* Array of pointers determined by header->length.
	* entries = (header->length - sizeof(sdth)) / 4
	*/
	uint32_t* pointers;
} __attribute__((packed)) rsdt_table;

/**
* Same as rsdt_table, but pointers are 64 bits. This is used in ACPI >= 2.0
*/
typedef struct	{
	sdth header;
	/**
	* Array of pointers determined by header->length.
	* entries = (header->length - sizeof(sdth)) / 8
	*/
	uint64_t* pointers;
} __attribute__((packed)) xsdt_table;


/**
* The structure of the Multiple APIC Description Table (MADT).
*/
typedef struct	{
	sdth header;
	uint32_t lapic_addr;
	uint32_t flags;
	// The rest are other structures, the first byte is the type and the second
	// is the length (in bytes?).
	// The total length is determined by header->length

} __attribute__((packed)) madt;

/**
* Header for many of the MADT structures.
*/
typedef struct	{
	uint8_t type;
	uint8_t length;
} __attribute__((packed)) madt_h;

/**
* Processor LAPIC structure. This must be present for each processor in the
* system.
*/
typedef struct	{
	madt_h h;
	uint8_t apic_pid;
	uint8_t apic_id;
	uint32_t flags;
} __attribute__((packed)) plapic;

/**
* I/O APIC structure. There is one of these for each I/O APIC controller.
*/
typedef struct	{
	madt_h h;

	/** Unique ID. */
	uint8_t ioapic_id;
	uint8_t reserved;
	uint32_t ioapic_addr;
	uint32_t global_sys_intr;
} __attribute__((packed)) ioapic;


/**
* Information the OS stores about the I/O APIC.
*/
typedef struct	{
	uint8_t id;
	uint32_t* addr;
	uint8_t max_interr;
} __attribute__((packed)) IO_apic;


/**
* Find, validate and store the RSDP descriptor.
* \returns Returns true if it is found and valid, false if it is not found or
* not valid.
*/
bool apic_init();


/**
* Find and store information about all the CPUs. The information required is the
* MADT table.
* \remark We don't know where the information is in memory, so this should be
* called while we are still using physical memory. After this we store
* everything we need in a known location.
* \returns Returns the number of CPUs found or 0 if the information is not
* available.
*/
int apic_find_cpus();

#endif

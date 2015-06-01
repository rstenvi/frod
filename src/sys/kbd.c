/**
* \file kbd.c
* Generic keyboard driver that handles the logic of the keypresses, but does not
* deal with hardware or the low-level packets.
*
* Design
* - Maintains a type of state machine that changes with the function
* kbd_add_scancode. The state consist of:
*  - Which key-modifiers are pressed, the same key-states that are included in
*  keyboard packet
*  - A sequence of scan codes not forwarded and pending interpretation.
* - Static data
*  - An array of all scancode to key_code mapping, scan code is index. If value
*  is found in this array, value can be forwarded.
*  - If the index is not found in the previous case we must store it.
*  - If we have a pending scan code and get a new one
*
* Key set is loaded as a block of memory with the following format:
* - X 32-bit elements with:
*  - 16-bit key code
*  - 16-bit flags matching some of the flags required in packet
*   - This includes make or break and type of key (numpad, etc)
* - If input is pending a new key, it has the following format
*  - (signed) 16-bit offset to new array
*  - 16-bit flag with only MSB set to 1.
*  - \todo I think this should be feasible
*/

#include "sys/kernel.h"
#include "sys/kbd.h"

sc_kc_map* orig_map;
sc_kc_map* curr_map;


void kbd_init(uint32_t keymap)	{
	orig_map = (sc_kc_map*)keymap;
	curr_map = (sc_kc_map*)keymap;
}


void kbd_add_scancode(uint8_t sc)	{
	if(curr_map->map[sc].flag & KBD_FLAG_MASK_PEND_MORE)	{
		uint32_t a = (uint32_t)orig_map;
		a += (curr_map->map[sc].kc)*256*sizeof(sc_kc);
		curr_map = (sc_kc_map*)a;
	}
	else	{
		kprintf(K_HIGH_INFO, "%x ", curr_map->map[sc].kc);
		curr_map = orig_map;
	}
}


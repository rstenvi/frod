/**
* \file ps2kbd.c
* PS/2 keyboard driver.
*
* Interact with the PS/2 driver to set up the connection and forwards the scan
* codes to the generic keyboard driver. This driver is responsible for setting
* up the HW (scan code set and verifying correctness) and notify the generig kbd
* of what scan code set is used as well as registering for interrupts.
*/

#include "sys/kernel.h"
#include "sys/kbd.h"
#include "drv/ps2.h"

#include "lib/stdio.h"


int32_t ps2kbd_init()	{
	ps2_register(kbd_add_scancode, "PS/2 Keyboard Driver", 1);
	return 0;
}



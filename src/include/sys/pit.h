/**
* \file pit.h
*/

#ifndef __PIT_H
#define __PIT_H

#include "kernel.h"

/**
* Initialize the PIT timer.
* \param[in] freq Number of ticks that should be sent per second.
*/
bool pit_install(uint32_t freq);


#endif


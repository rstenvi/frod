/**
* \file dllist.h
* Implementation of a standard doubly linked list to hold arbitrary data, as
* long as the caller specify some comparison functions.
*
* Is only meant to store elements that have an unique ID, but it doesn't enforce
* any checks.
*/


#ifndef __DLLIST_H
#define __DLLIST_H

#include "kernel.h"




typedef int8_t (*dllist_lessthan)(void*,void*);
//typedef int8_t (*dllist_lessthan2)(void*,void*);

typedef struct _element	{
	void* element;
	struct _element* prev, * next;

} dllist_element;


typedef struct _dllist	{
	dllist_lessthan lessthan;
	dllist_lessthan lessthan_find;

	dllist_element* elem;

	uint32_t elements;
} dllist_head;



/**
* Initialize a new list.
* \param[in] el Initial data, can be NULL
* \param[in] f1 Comparison function for two objects
* \param[in] f2 Comparison function for 1 object and 1 key value
* \returns Returns the object that should be used in future calls
*/
dllist_head* dllist_init(void* el, dllist_lessthan f1, dllist_lessthan f2);


void dllist_insert(dllist_head* l, void* el);

void dllist_insert_end(dllist_head* l, void* el);

/**
* Find an element based on a value, this is the second compare function.
*/
void* dllist_find(dllist_head* l, void* val);


/**
* Rmove and element from the list based on the ID value.
* \remark Function frees memory allocated here and returns the pointer, so
* caller can do whatever he wants with it, which will usually be free it.
*/
void* dllist_remove(dllist_head* l, void* val);

#endif

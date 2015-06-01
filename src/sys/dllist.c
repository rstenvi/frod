

#include "sys/kernel.h"
#include "sys/dllist.h"
#include "sys/heap.h"

void dllist_insert_location(dllist_element* bef, void* el);





dllist_head* dllist_init(void* el, dllist_lessthan f1, dllist_lessthan f2)	{
	dllist_head* ret = heap_malloc(sizeof(dllist_head));
	dllist_element* elem = heap_malloc(sizeof(dllist_element));
	elem->element = el;
	ret->elem = elem;
	ret->elem->prev = ret->elem;
	ret->elem->next = ret->elem;

	ret->lessthan = f1;
	ret->lessthan_find = f2;

	ret->elements = ((el == NULL) ? 0 : 1);

	return ret;
}



void dllist_insert(dllist_head* l, void* el)	{
	dllist_element* curr = l->elem;
	dllist_element* prev;

	// Function never (intentionally) fails, so we can just increment
	// immediately
	l->elements++;

	// If the list is empty we just insert it
	if(curr == NULL)	{
		l->elem = heap_malloc(sizeof(dllist_element));
		l->elem->element = el;
		return;
	}

	do	{
		// If el should come before current
		if(l->lessthan(el, curr->element) > 0)	{
			prev = curr->prev;
			dllist_insert_location(prev, el);

			// Extra case if this is the new first element
			if(l->elem == curr)	{
				l->elem = prev;
			}
			
			return;
		}
		curr = curr->next;
	} while(l->elem != curr);

	// If we get here, it should be placed at the end of the list
	// curr is the start
	prev = curr->prev;
	dllist_insert_location(prev, el);
}

void dllist_insert_end(dllist_head* l, void* el)	{
	dllist_element* curr = l->elem;
	dllist_element* n = heap_malloc(sizeof(dllist_element));
	n->element = el;
	l->elements++;
	// If the list is empty we just insert it
	if(curr == NULL)	{
		l->elem->element = el;
	}
	else	{
		curr = curr->prev;
		n->next = curr->next;
		curr->next = n;
	}
}

void* dllist_find(dllist_head* l, void* val)	{
	dllist_element* curr = l->elem;
	if(curr == NULL)	return NULL;
	int8_t cmp = 0;
	do	{
		if((cmp = l->lessthan_find(curr->element, val)) == 0)
			return curr->element;
		curr = curr->next;
	} while(cmp <= 0);
	return NULL;
}

void* dllist_remove(dllist_head* l, void* val)	{
	void* ret = NULL;
	dllist_element* elem = dllist_find(l, val);
	if(elem == NULL)	return NULL;
	
	elem->prev->next = elem->next;
	ret = elem->element;

	// Check if we just removed the last element in the list
	if(l->elem == elem)	{
		l->elem = NULL;
	}
	heap_free(elem);
	return ret;
}





//---------------- Internal functions ---------------------------

void dllist_insert_location(dllist_element* bef, void* el)	{
	dllist_element* elem = heap_malloc(sizeof(dllist_element));
	elem->element = el;

	// Insert after current element
	elem->next = bef->next;
	bef->next = elem;
	elem->prev = bef;

}



#ifdef TEST_KERNEL

typedef struct	{
	int id, value;
} dllist_test_int;






#endif

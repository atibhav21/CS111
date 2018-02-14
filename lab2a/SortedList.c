#include "SortedList.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	SortedListElement_t* iterator = list->next;
	while(iterator != list && strcmp(iterator-> key, element-> key) <= 0)
	{
		// Start of Critical Section
		if((opt_yield & INSERT_YIELD) != 0)
			sched_yield();
		iterator = iterator->next;
		// End of critical section
	}
	// need to insert element before where iterator is right now.
	// Start of Critical Section
	if((opt_yield & INSERT_YIELD) != 0)
		sched_yield();
	element->prev = iterator->prev;
	element->next = iterator;
	iterator->prev->next = element;
	iterator->prev = element;

	// End of critical section
}


int SortedList_delete( SortedListElement_t *element)
{
	if(element->prev->next == element && element->next->prev == element)
	{
		// everything fine so delete the element

		// Start of Critical Section
		if((opt_yield & DELETE_YIELD) != 0)
			sched_yield();
		element->prev->next = element->next;
		element->next->prev = element->prev;
		//free(element);
		// End of Critical Section
		return 0;
	}

	return 1;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	SortedListElement_t* iterator = list-> next;
	while(iterator != list)
	{
		if(strcmp(iterator->key, key) == 0)
		{
			return iterator;
		}
		// Start of Critical Section
		if((opt_yield & LOOKUP_YIELD) != 0)
			sched_yield();
		iterator = iterator-> next;
		// End of Critical Section
	}
	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	SortedListElement_t* iterator = list -> next;
	int count = 0;
	while(iterator != list)
	{
		if(iterator->prev->next != iterator || iterator ->next -> prev != iterator)
		{
			// corrupted list
			return -1;
		}
		if((opt_yield & LOOKUP_YIELD) != 0)
			sched_yield();
		count += 1;
		iterator = iterator -> next;
	}
	return count;
}



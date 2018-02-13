#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	SortedListElement_t* iterator = list;
	SortedListElement_t* last_element_processed = NULL;
	while(iterator != NULL)
	{
		if(iterator-> key < element-> key)
		{
			// move iterator to next element
			last_element_processed = iterator;
			iterator = iterator-> next;
		}
		else if(iterator->key >= element->key)
		{	
			// Start of critical section??

			// insert before where the iterator is and then return
			element->next = iterator;
			element->prev = iterator-> prev;
			if(iterator -> prev != NULL)
			{
				iterator-> prev -> next = element;
				
			}
			else
			{
				// insert before first element
				iterator-> prev = element;
			}
			return;
		}
	}
	// Another Critical Section

	// insert at the end of the list
	last_element_processed -> next = element;
	element -> prev = last_element_processed;
	element -> next = NULL;

}


int SortedList_delete( SortedListElement_t *element)
{
	// Start of Critical Section, can put the entire thing in a critical section since it runs in O(1) time

	if(element-> prev != NULL)
	{
		if(element->prev->next != element)
		{
			return 1;
		}
	}
	if(element->next != NULL)
	{
		if(element->next->prev != element)
		{
			return 1;
		}
	}
	// otherwise change pointers and remove element
	if(element-> prev != NULL)
	{
		element->prev->next = element->next;
	}
	if(element->next != NULL)
	{
		element->next->prev = element->prev;
	}

	free(element);

	// End of Critical Section

	return 0;

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	SortedListElement_t* iterator = list;
	while(iterator != NULL)
	{
		// Start of Critical Section
		if(strcmp(iterator->key, key) == 0)
		{
			return iterator;
		}
		iterator = iterator-> next;
		// End of Critical Section
	}
	return NULL;
}

int SortedList_length(SortedList_t *list)
{
	SortedListElement_t* iterator = list;
	if(iterator == NULL)
	{
		// empty list!!
		return 0;
	}
	int count = 0;
	while(iterator != NULL)
	{
		if(iterator-> prev != NULL && iterator-> prev -> next != iterator)
		{
			// corrupted list
			return -1;
		}
		if(iterator-> next != NULL && iterator->next->prev != iterator)
		{
			// corrupted list
			return -1;
		}
		else
		{
			// Start of Critical section
			count += 1;
			iterator = iterator -> next;
			// End of critical section
		}
	}

	return count;
}




































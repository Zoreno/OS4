#ifndef _LIST_H
#define _LIST_H

#include <lib/stdint.h>

/**
 * Double linked list element
 */
typedef struct _dl_element_t
{
	/**
	 * Pointer to the actual data
	 */
	void*				data;

	/**
	 * Pointer to the next list element
	 */
	struct _dl_element_t*	next;

	/**
	 * Pointer to the previous element
	 */
	struct _dl_element_t* prev;
	
} dl_element_t;

/**
 * Double linked list structure
 */
typedef struct _list_t
{
	/**
 	 * First node in the list
 	 */
	dl_element_t* head;

	/**
	 * Last node in the list
	 */
	dl_element_t* tail;
} list_t;

/**
 * @brief Creates an empty list
 *
 * @return Pointer to list 
 */
list_t* list_create();

/**
 * @brief Initializes a list
 *
 * @param list Pointer to list 
 */
void list_construct(list_t* list);
void list_destruct(list_t* list);
void list_free(list_t* list);
dl_element_t* list_alloc_elem(size_t size);
dl_element_t* list_insert(list_t* list, dl_element_t* next, void* data);
dl_element_t* list_append(list_t* list, void* data);
void list_append_elem(list_t* list, dl_element_t* elem);
dl_element_t* list_delete(list_t* list, dl_element_t* elem);
dl_element_t* list_get_element(list_t* list, size_t num);
dl_element_t* list_find(const list_t* list, void* data);
size_t  list_get_count(const list_t* list);
int		list_is_empty(const list_t* list);

#endif

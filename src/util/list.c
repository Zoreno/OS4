#include <util/list.h>
#include <lib/string.h>

list_t* list_create()
{
	list_t* list = kmalloc(sizeof(list_t));
	if(list)
	{
		list->head = 0;
		list->tail = 0;
	}

	return list;
}

void list_construct(list_t* list)
{
	list->head = 0;
	
	list->tail = 0;
}

void list_destruct(list_t* list)
{
	dl_element_t* cur = list->head;

	while(cur)
	{
		dl_element_t* next = cur->next;
		// kfree(cur->data);
		kfree(cur);
		cur = next;
	}

	list->head = 0;
	list->tail = 0;
}

void list_free(list_t* list)
{
	list_destruct(list);
	kfree(list);
}

dl_element_t* list_alloc_elem(size_t size)
{
	dl_element_t* newElement = kmalloc(sizeof(dl_element_t) + size);

	if(newElement)
		newElement->data = newElement + 1;

	return newElement;
}

dl_element_t* list_insert(list_t* list, dl_element_t* next, void* data)
{
	if(next == 0)
	{
		return list_append(list, data);
	}

	dl_element_t* newElement = kmalloc(sizeof(dl_element_t));

	if(newElement)
	{
		newElement->data = data;

		if(next == list->head)
		{
			newElement->next = list->head;
			newElement->prev = 0;
			list->head->prev = newElement;
			list->head = newElement;
		}
		else
		{
			newElement->prev = next->prev;
			newElement->next = next;
			next->prev->next = newElement;
			next->prev = newElement;
		}

		return newElement;
	}

	return 0;
}

dl_element_t* list_append(list_t* list, void* data)
{
	dl_element_t* newElement = kmalloc(sizeof(dl_element_t));

	if(newElement)
	{
		newElement->data = data;
		list_append_elem(list, newElement);
	}

	return newElement;
}

void list_append_elem(list_t* list, dl_element_t* elem)
{
	elem->next = 0;
	elem->prev = list->tail;

	if(list->head == 0)
	{
		list->head = elem;
	}
	else
	{
		list->tail->next = elem;
	}

	list->tail = elem;
}

dl_element_t* list_delete(list_t* list, dl_element_t* elem)
{
	if(list->head == 0)
	{
		return 0;
	}

	if(list->head == list->tail)
	{
		kfree(elem);
		list->head = list->tail = 0;
		return 0;
	}

	dl_element_t* temp = elem->next;

	if(elem == list->head)
	{
		list->head = elem->next;
		list->head->prev = 0;
	}
	else if (elem == list->tail)
	{
		list->tail = elem->prev;
		list->tail->next = 0;
	}
	else
	{
		elem->prev->next = elem->next;
		elem->next->prev = elem->prev;
	}

	kfree(elem);

	return temp;
			
}

dl_element_t* list_get_element(list_t* list, size_t num)
{
	dl_element_t* cur = list->head;

	while(1)
	{
		if(num == 0 || cur == 0)
		{
			return cur;
		}

		--num;
		cur = cur->next;
	}
}

dl_element_t* list_find(const list_t* list, void* data)
{
	dl_element_t* cur = list->head;
	while(cur && cur->data != data)
	{
		cur = cur->next;
	}

	return cur;
}

size_t  list_get_count(const list_t* list)
{
	size_t count = 0;

	for(dl_element_t* e = list->head; e; e = e->next)
	{
		++count;
	}
	return count;
}

int	list_is_empty(const list_t* list)
{
	return list->head == 0;
}

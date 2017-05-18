#include <gui/rect.h>

#include <lib/string.h>
#include <lib/stdio.h>

Rect* Rect_New(
	int top,
	int left,
	int bottom,
	int right)
{
	// serial_printf(COM1, "Creating Rect\n");
	Rect* rect;
	rect = (Rect*)kmalloc(sizeof(Rect));

	if(!rect)
		return rect;

	rect->top = top;
	rect->left = left;
	rect->bottom = bottom;
	rect->right = right;

	return rect;
}

list_t* Rect_Split(
	Rect* subject_rect,
	Rect* cutting_rect)
{
	// serial_printf(COM1, "Splitting Rect\n");
	// Allocate list to store result
	list_t* output_rects;
	output_rects = list_create();
	if(!output_rects)
		return output_rects;

	// Copy input so we can modify it
	Rect subject_copy;
	subject_copy.top = subject_rect->top;
	subject_copy.left = subject_rect->left;
	subject_copy.right = subject_rect->right;
	subject_copy.bottom = subject_rect->bottom;

	Rect* temp_rect;

	// Split by left edge
	
	if(cutting_rect->left >= subject_copy.left &&
	   cutting_rect->left <= subject_copy.right)
	{
		// Create a new rectangle
		temp_rect = Rect_New(
			subject_copy.top,
			subject_copy.left,
			subject_copy.bottom,
			cutting_rect->left - 1);

		// Make sure allocation succeeded.
		if(!temp_rect)
		{
			kfree(output_rects);
			return (list_t*)0;
		}

		// Append new rectangle to list.
		list_append(output_rects, temp_rect);

		// Shrink subject to exclude the removed portion.
		subject_copy.left = cutting_rect->left;
	}

	// Split by top edge

	if(cutting_rect->top >= subject_copy.top &&
	   cutting_rect->top <= subject_copy.bottom)
	{
		// Create a new rectangle
		temp_rect = Rect_New(
			subject_copy.top,
			subject_copy.left,
			cutting_rect->top - 1,
			subject_copy.right);

		// Make sure allocation succeeded.
		if(!temp_rect)
		{
			while(list_get_count(output_rects))
			{
				dl_element_t* elem = list_get_element(output_rects, 0);
				kfree(elem->data);
				list_delete(output_rects, elem);
			}
			
			kfree(output_rects);
			return (list_t*)0;
		}

		// Append new rectangle to list.
		list_append(output_rects, temp_rect);

		// Shrink subject to exclude the removed portion.
		subject_copy.top = cutting_rect->top;
	}

	// Split by right edge

	if(cutting_rect->right >= subject_copy.left &&
	   cutting_rect->right <= subject_copy.right)
	{
		// Create a new rectangle
		temp_rect = Rect_New(
			subject_copy.top,
			cutting_rect->right + 1,
		    subject_copy.bottom,
			subject_copy.right);

		// Make sure allocation succeeded.
		if(!temp_rect)
		{
			while(list_get_count(output_rects))
			{
				dl_element_t* elem = list_get_element(output_rects, 0);
				kfree(elem->data);
				list_delete(output_rects, elem);
			}
			
			kfree(output_rects);
			return (list_t*)0;
		}

		// Append new rectangle to list.
		list_append(output_rects, temp_rect);

		// Shrink subject to exclude the removed portion.
		subject_copy.right = cutting_rect->right;
	}

	// Split by bottom edge

	if(cutting_rect->bottom >= subject_copy.top &&
	   cutting_rect->bottom <= subject_copy.bottom)
	{
		// Create a new rectangle
		temp_rect = Rect_New(
			cutting_rect->bottom + 1,
			subject_copy.left,
		    subject_copy.bottom,
			subject_copy.right);

		// Make sure allocation succeeded.
		if(!temp_rect)
		{
			while(list_get_count(output_rects))
			{
				dl_element_t* elem = list_get_element(output_rects, 0);
				kfree(elem->data);
				list_delete(output_rects, elem);
			}
			
			kfree(output_rects);
			return (list_t*)0;
		}

		// Append new rectangle to list.
		list_append(output_rects, temp_rect);

		// Shrink subject to exclude the removed portion.
		subject_copy.bottom = cutting_rect->bottom;
	}

	return output_rects;
}

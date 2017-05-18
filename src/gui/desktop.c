#include <gui/desktop.h>

#include <lib/string.h>
#include <lib/stdio.h>

#define DEBUG_DESKTOP 0

void Desktop_ProcessMouse(
	Desktop* desktop,
	uint16_t mouse_x,
	uint16_t mouse_y,
	uint8_t mouse_buttons)
{
	int i;
	Window* child;

	// Update our mouse positon
	desktop->mouse_x = mouse_x;
	desktop->mouse_y = mouse_y;

	// Check mouse status
	if(mouse_buttons)
	{
		// If it was down last time, it has now been pressed
		if(!desktop->last_button_state)
		{
			// Iterate the list of windows backwards
			for(i = list_get_count(desktop->children) - 1; i >= 0; --i)
			{
				// Get current window
				dl_element_t* elem = list_get_element(desktop->children, i);
				child = (Window*)elem->data;

				// Check if mouse is inside window title bar
				if(mouse_x >= child->x &&
				   mouse_x < (child->x + child->width) &&
				   mouse_y >= child->y &&
				   mouse_y < (child->y + 31))
				{
					// Remove window and reinsert it.
					list_delete(desktop->children, elem);
					list_append(desktop->children, (void*)child);

					desktop->drag_off_x = mouse_x - child->x;
					desktop->drag_off_y = mouse_y - child->y;
					desktop->drag_child = child;
					
					break;
				}
			}
		}
	}
	else
	{
		desktop->drag_child = (Window*)0;
	}

	if(desktop->drag_child)
	{
		desktop->drag_child->x = mouse_x - desktop->drag_off_x;
		desktop->drag_child->y = mouse_y - desktop->drag_off_y;
	}

	Desktop_Paint(desktop);

	desktop->last_button_state = mouse_buttons;
}

Desktop* Desktop_Create(
	Rendering_Context* context)
{
	Desktop* desktop;

	desktop = (Desktop*) kmalloc(sizeof(Desktop));

	if(!desktop)
		return desktop;

	desktop->children = list_create();

	if(!desktop->children)
	{
		kfree(desktop);
		return (Desktop*)0;
	}

	desktop->context = context;

	desktop->last_button_state = 0;
	desktop->mouse_x = 20;
	desktop->mouse_y = 20;

	return desktop;
}

Window* Desktop_Create_Window(
	Desktop* desktop,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height)
{

#if DEBUG_DESKTOP == 1
	serial_printf(COM1, "[DT] Creating Window\n");
#endif
	
	Window* window;

	window = Window_Create(
		x,
		y,
		width,
		height,
		desktop->context);

	list_append(desktop->children, window);

	return window;
}

void Desktop_Paint_Old(
	Desktop* desktop)
{
	unsigned int i;
	Window* current_window;
	Rect* temp_rect;
	
	Rendering_Context_Fill_Rect(
		desktop->context,
		0,
		0,
		desktop->context->width,
		desktop->context->height,
		0xFF9933FF);

	Rendering_Context_Clear_Clip_Rects(desktop->context);

	dl_element_t* curr_elem;
	
	for(i = 0; (curr_elem = list_get_element(desktop->children, i)); ++i)
	{
		current_window = (Window*)curr_elem->data;

		temp_rect = Rect_New(
			current_window->y,
			current_window->x,
			current_window->y + current_window->height - 1,
			current_window->x + current_window->width - 1);

	   	Rendering_Context_Add_Clip_Rect(desktop->context, temp_rect);
	}
	
	size_t length = list_get_count(desktop->context->clip_rects);
	
	for(size_t i = 0; i < length; ++i)
	{
	    temp_rect = (Rect*)list_get_element(desktop->context->clip_rects, i)->data;

		Rendering_Context_Draw_Rect(
			desktop->context,
			temp_rect->left,
			temp_rect->top,
			temp_rect->right - temp_rect->left + 1,
			temp_rect->bottom - temp_rect->top + 1,
			0xFF00FF00);
	}
	

	// Draw Mouse
	Rendering_Context_Fill_Rect(
		desktop->context,
		desktop->mouse_x,
		desktop->mouse_y,
		10,
		10,
		0xFF000000);
}

void Desktop_Paint_Old2(
	Desktop* desktop)
{

#if DEBUG_DESKTOP == 1
	serial_printf(COM1, "[DT] Painting Desktop\n");
#endif
	
	unsigned int i;
	Window* current_window;
	Rect* temp_rect;

	// Render Each Quadrant
	Rendering_Context_Fill_Rect(
		desktop->context,
		0,
		0,
		desktop->context->width / 2,
		desktop->context->height / 2,
		0xFF0000FF);

	Rendering_Context_Fill_Rect(
		desktop->context,
		desktop->context->width / 2,
		0,
		desktop->context->width / 2,
		desktop->context->height / 2,
		0xFF00FF00);

	Rendering_Context_Fill_Rect(
		desktop->context,
		0,
		desktop->context->height / 2,
		desktop->context->width / 2,
		desktop->context->height / 2,
		0xFF00FFFF);

	Rendering_Context_Fill_Rect(
		desktop->context,
		desktop->context->width / 2,
		desktop->context->height / 2,
		desktop->context->width / 2,
		desktop->context->height / 2,
		0xFFFF00FF);

	temp_rect = Rect_New(
		0,
		0,
		desktop->context->height - 1,
		desktop->context->width - 1);

	Rendering_Context_Add_Clip_Rect(
		desktop->context,
		temp_rect);

	dl_element_t* elem;

	size_t elem_count = list_get_count(desktop->children);

	for(i = 0; i < elem_count; ++i)
	{
		elem = list_get_element(desktop->children, i);
		current_window = elem->data;
		temp_rect = Rect_New(
			current_window->y,
			current_window->x,
			current_window->y + current_window->height - 1,
			current_window->x + current_window->width - 1);

		Rendering_Context_Subtract_Clip_Rect(
			desktop->context,
			temp_rect);

		//kfree(temp_rect);
	}

	Rendering_Context_Fill_Rect(
		desktop->context,
		0,
		0,
		desktop->context->width,
		desktop->context->height,
		0xFFFF9933);

	// Clear context
	Rendering_Context_Clear_Clip_Rects(
		desktop->context);

	// Draw Mouse
	Rendering_Context_Fill_Rect(
		desktop->context,
		desktop->mouse_x,
		desktop->mouse_y,
		10,
		10,
		0xFF000000);
}

void Desktop_Paint(
	Desktop* desktop)
{
	unsigned int i;
	unsigned int j;
	Window* current_window;
	Window* clipping_window;

	Rect* temp_rect;
	list_t* clip_windows;

	// Create an initial rect for the desktop to ensure we
	// do not draw out of bounds.
	temp_rect = Rect_New(
		0,
		0,
		desktop->context->height - 1,
		desktop->context->width - 1);

	// Add it
	Rendering_Context_Add_Clip_Rect(
		desktop->context,
		temp_rect);

	size_t window_count = list_get_count(desktop->children);

	// Find all rects blocking desktop
	for(i = 0; i < window_count; ++i)
	{
		current_window = (Window*)list_get_element(desktop->children, i)->data;

		temp_rect = Rect_New(
			current_window->y,
			current_window->x,
			current_window->y + current_window->height - 1,
			current_window->x + current_window->width - 1);

		Rendering_Context_Subtract_Clip_Rect(
			desktop->context,
			temp_rect);

		kfree(temp_rect);
	}

	// Draw the desktop
	Rendering_Context_Fill_Rect(
		desktop->context,
		0,
		0,
		desktop->context->width,
		desktop->context->height,
		0xFFFF99339);

	// Reset context clipping
	Rendering_Context_Clear_Clip_Rects(
		desktop->context);

	// Draw each window
	for(i = 0; i < window_count; ++i)
	{
		current_window = (Window*)list_get_element(desktop->children, i)->data;

		temp_rect = Rect_New(
			current_window->y,
			current_window->x,
			current_window->y + current_window->height - 1,
			current_window->x + current_window->width - 1);

		// Add clip rect to context
		Rendering_Context_Add_Clip_Rect(
		    desktop->context,
			temp_rect);

		// Find all clipping windows
		clip_windows = Desktop_Get_Windows_Above(
			desktop,
			current_window);

		// Get all clipping geometry
		while(list_get_count(clip_windows))
		{
			dl_element_t* elem = list_get_element(clip_windows, 0);
			clipping_window = elem->data;
			list_delete(clip_windows, elem);

			if(clipping_window == current_window)
				continue;

			temp_rect = Rect_New(
				clipping_window->y,
				clipping_window->x,
				clipping_window->y + clipping_window->height - 1,
				clipping_window->x + clipping_window->width - 1);

			Rendering_Context_Subtract_Clip_Rect(
				desktop->context,
				temp_rect);

			kfree(temp_rect);
		}

		// Do the actual painting of the window
		Window_Paint(current_window);

		kfree(clip_windows);
		Rendering_Context_Clear_Clip_Rects(
			desktop->context);
	}


	// Simple mouse

	Rendering_Context_Fill_Rect(
		desktop->context,
		desktop->mouse_x,
		desktop->mouse_y,
		10,
		10,
		0xFF000000);
}

list_t* Desktop_Get_Windows_Above(
	Desktop* desktop,
	Window* window)
{
	int i;
	Window* current_window;
	list_t* return_list;

	return_list = list_create();

	if(!return_list)
		return return_list;

	size_t windows_count = list_get_count(desktop->children);

	for(i = 0; i < windows_count; ++i)
	{
		if(window == (Window*)list_get_element(desktop->children, i)->data)
			break;
	}

	for(; i < windows_count; ++i)
	{
		current_window = (Window*)list_get_element(desktop->children, i)->data;

		if(current_window->x <= (window->x + window->width - 1) &&
		   (current_window->x + current_window->width - 1) >= window->x &&
		   current_window->y <= (window->y + window->height - 1) &&
		   (current_window->y + current_window->height - 1) >= window->y)
		{
			list_append(return_list, current_window);
		}
	}

	return return_list;
}

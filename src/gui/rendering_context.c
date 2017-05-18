#include <gui/rendering_context.h>

#include <lib/string.h>
#include <lib/stdio.h>

#define DEBUG_RENDERING_CONTEXT 0

Rendering_Context* Rendering_Context_New(
	uint32_t width,
	uint32_t height,
	uint32_t* buffer)
{
#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Creating Rendering Context\n");
#endif
	
	// Allocate a context struct
	Rendering_Context* context;
	context = (Rendering_Context*)kmalloc(sizeof(Rendering_Context));
	if(!context)
		return context;

	// Create clipping rects list.
	context->clip_rects = list_create();

	if(!context->clip_rects)
	{
		kfree(context);
		return (Rendering_Context*)0;
	}

	// Initialize the rest of the parameters
	context->width = width;
	context->height = height;
	context->buffer = buffer;

	return context;
}

void Rendering_Context_Subtract_Clip_Rect(
	Rendering_Context* context,
	Rect* subtracted_rect)
{
#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Subtracting Clip Rect\n");
#endif
	
	int i;
	int j;
	Rect* cur_rect;
	list_t* split_rects;

	// Check all current items on list
	for(i = 0; i < list_get_count(context->clip_rects);)
	{
		cur_rect = list_get_element(context->clip_rects, i)->data;
		// Check if rectangle intersects.
		if(!(cur_rect->left <= subtracted_rect->right &&
			 cur_rect->right >= subtracted_rect->left &&
			 cur_rect->top <= subtracted_rect->bottom &&
			 cur_rect->bottom >= subtracted_rect->top))
		{
			// If not, continue
			++i;
			continue;
		}

	   	// Remove and split
		list_delete(context->clip_rects, list_get_element(context->clip_rects, i));
		split_rects = Rect_Split(cur_rect, subtracted_rect);
		kfree(cur_rect);

		while(list_get_count(split_rects))
		{
			dl_element_t* elem = list_get_element(split_rects, 0);
			cur_rect = (Rect*) elem->data;
			list_delete(split_rects, elem);
			list_append(context->clip_rects, cur_rect);
		}

		kfree(split_rects);

		i = 0;
	}
}

void Rendering_Context_Add_Clip_Rect(
	Rendering_Context* context,
	Rect* added_rect)
{

#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Adding Clip Rect\n");
#endif
	
	Rendering_Context_Subtract_Clip_Rect(
		context,
		added_rect);

	list_append(context->clip_rects, added_rect);
}

void Rendering_Context_Clear_Clip_Rects(
	Rendering_Context* context)
{
	
#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Clearing Clip Rects\n");
#endif
	
	Rect* cur_rect;

	while(list_get_count(context->clip_rects))
	{
		dl_element_t* elem = list_get_element(context->clip_rects, 0);
		cur_rect = elem->data;
		kfree(cur_rect);

		list_delete(context->clip_rects, elem);
	}
}

void Rendering_Context_Clipped_Rect(
	Rendering_Context* context,
	int32_t x,
	int32_t y,
	uint32_t width,
	uint32_t height,
	Rect* clip_area,
	uint32_t color)
{
#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Drawing Clip Rect\n");
	serial_printf(COM1, "[RC] Parameters: x: %i, y: %i, width: %i, height: %i\n",
				  x, y, width, height);
	serial_printf(COM1,
				  "[RC] Clip Area: left: %i, top: %i, right: %i, bottom: %i\n",
				  clip_area->left,
				  clip_area->top,
				  clip_area->right,
				  clip_area->bottom);
#endif
	
	int32_t cur_x;
	int32_t max_x = x + width;
	int32_t max_y = y + height;

	if(x < clip_area->left)
		x = clip_area->left;

	if(y < clip_area->top)
		y = clip_area->top;

	if(max_x > clip_area->right + 1)
		max_x = clip_area->right + 1;

	if(max_y > clip_area->bottom + 1)
		max_y = clip_area->bottom + 1;

#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Area: x: %i, y: %i, max_x: %i, max_y: %i\n",
				  x, y, max_x, max_y);
#endif

	
	uint32_t* buffer = &context->buffer[y * context->width + x];

	int32_t visible_width = max_x - x;

	if(visible_width <= 0)
	{
		// Nothing to render
		return;
	}
	
	for(; y < max_y; ++y)
	{
		
		memsetl(buffer, color, visible_width);

		buffer += context->width;	
	}
	
	/*
	for(; y < max_y; ++y)
	{
		for(cur_x = x; cur_x < max_x; ++cur_x)
		{
			context->buffer[y * context->width + cur_x] = color;
		}
	}
	*/
}

void Rendering_Context_Fill_Rect(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	uint32_t color)
{

#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Filling Rect\n");
	serial_printf(COM1, "[RC] Parameters: x: %i, y: %i, width: %i, height: %i\n",
				  x, y, width, height);
#endif
	
	int32_t start_x;
	int32_t cur_x;
	int32_t cur_y;
	int32_t end_x;
	int32_t end_y;

	int32_t max_x = x + width;
	int32_t max_y = y + height;

	int32_t i;

	Rect* clip_area;
	Rect screen_area;

	int32_t clip_count = list_get_count(context->clip_rects);
	
	if(clip_count)
	{
		for(i = 0; i < clip_count; ++i)
		{
			clip_area = (Rect*)list_get_element(context->clip_rects, i)->data;
			
			Rendering_Context_Clipped_Rect(
				context,
				x,
				y,
				width,
				height,
				clip_area,
				color);
		}
	}
	else
	{
		screen_area.top = 0;
		screen_area.left = 0;
		screen_area.bottom = context->height - 1;
		screen_area.right = context->width - 1;

		Rendering_Context_Clipped_Rect(
			context,
			x,
			y,
			width,
			height,
			&screen_area,
			color);
	}
}

void Rendering_Context_Fill_Rect_Old(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	uint32_t color)
{	
	uint32_t cur_x;
	uint32_t max_x;
	uint32_t max_y;

	max_x = x + width;
	max_y = y + height;

	if(max_x > context->width)
	{
		max_x = context->width;
	}

	if(max_y > context->height)
	{
		max_y = context->height;
	}

	uint32_t* buffer = &context->buffer[y * context->width + x];

	int32_t visible_width = max_x - x;

	if(visible_width <= 0)
	{
		// Nothing to render
		return;
	}
	
	for(; y < max_y; ++y)
	{

		memsetl(buffer, color, visible_width);

		buffer += context->width;
		
		/*
		for(cur_x = x; cur_x < max_x; ++cur_x)
		{
			context->buffer[y * context->width + cur_x] = color;
		}
		*/
	}
}

void Rendering_Context_Draw_Rect(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	uint32_t color)
{
	
#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Drawing Rect\n");
#endif
	
	// Top Line
	Rendering_Context_Draw_Horizontal_Line(
		context,
		x,
		y,
		width,
		color);

	// Left
	Rendering_Context_Draw_Vertical_Line(
		context,
		x,
		y + 1,
		height - 2,
		color);

	// Bottom
	Rendering_Context_Draw_Horizontal_Line(
		context,
		x,
		y + height - 1,
		width,
		color);

	// Right
	Rendering_Context_Draw_Vertical_Line(
		context,
		x + width - 1,
		y + 1,
		height - 2,
		color);
}

void Rendering_Context_Draw_Horizontal_Line(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t length,
	uint32_t color)
{

#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Drawing Horizontal line\n");
#endif
	
	Rendering_Context_Fill_Rect(
		context,
		x,
		y,
		length,
		1,
		color);
}

void Rendering_Context_Draw_Vertical_Line(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t length,
	uint32_t color)
{

#if DEBUG_RENDERING_CONTEXT == 1
	serial_printf(COM1, "[RC] Drawing vertical line\n");
#endif
	
	Rendering_Context_Fill_Rect(
		context,
		x,
		y,
		1,
		length,
		color);
}

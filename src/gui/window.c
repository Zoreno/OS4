#include <gui/window.h>

#include <gui/rendering_context.h>

#include <lib/string.h>

uint8_t rand_8()
{
	static uint16_t seed = 0;
	return (uint8_t)(seed = (12657 * seed + 12345) % 256);
}

Window* Window_Create(
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	Rendering_Context* context)
{
	Window* window;

	window = (Window*)kmalloc(sizeof(Window));

	if(!window)
		return window;

	window->x = x;
	window->y = y;
	window->width = width;
	window->height = height;

	window->context = context;
	
	return window;
}

void Window_Paint(Window* window)
{
	// Draw 3 px border
	
   	Rendering_Context_Draw_Rect(
		window->context,
		window->x,
		window->y,
		window->width,
		window->height,
		WIN_BORDERCOLOR);

	Rendering_Context_Draw_Rect(
		window->context,
		window->x + 1,
		window->y + 1,
		window->width - 2,
		window->height - 2,
		WIN_BORDERCOLOR);

	Rendering_Context_Draw_Rect(
		window->context,
		window->x + 2,
		window->y + 2,
		window->width - 4,
		window->height - 4,
		WIN_BORDERCOLOR);

	// Draw 3 px line under titlebar

	Rendering_Context_Draw_Horizontal_Line(
		window->context,
		window->x + 3,
		window->y + 28,
		window->width - 6,
		WIN_BORDERCOLOR);

	Rendering_Context_Draw_Horizontal_Line(
		window->context,
		window->x + 3,
		window->y + 29,
		window->width - 6,
		WIN_BORDERCOLOR);

	Rendering_Context_Draw_Horizontal_Line(
		window->context,
		window->x + 3,
		window->y + 30,
		window->width - 6,
		WIN_BORDERCOLOR);

	// Fill in titlebar background

	Rendering_Context_Fill_Rect(
		window->context,
		window->x + 3,
		window->y + 3,
		window->width - 6,
		25,
		WIN_TITLECOLOR);

	// Fill in window background

	Rendering_Context_Fill_Rect(
		window->context,
		window->x + 3,
		window->y + 31,
		window->width - 6,
		window->height - 34,
		WIN_BGCOLOR);
}


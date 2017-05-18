#ifndef _WINDOW_H
#define _WINDOW_H

#include <gui/rendering_context.h>

#include <lib/stdint.h>

#define WIN_BGCOLOR		0xFFBBBBBB
#define WIN_TITLECOLOR	0xFFBE9270
#define WIN_BORDERCOLOR	0xFF000000

typedef struct _Window
{
	uint32_t x;
	uint32_t y;
	uint32_t width;
	uint32_t height;

	Rendering_Context* context;
} Window;

Window* Window_Create(
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	Rendering_Context* context);

void Window_Paint(
	Window* window);

#endif

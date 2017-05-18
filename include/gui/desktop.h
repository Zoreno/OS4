#ifndef _DESKTOP_H
#define _DESKTOP_H

#include <util/list.h>

#include <gui/rendering_context.h>
#include <gui/window.h>

#include <lib/stdint.h>

typedef struct _Desktop
{
	list_t* children;
	Rendering_Context* context;
	
	uint8_t last_button_state;
	uint16_t mouse_x;
	uint16_t mouse_y;

	Window* drag_child;
	uint16_t drag_off_x;
	uint16_t drag_off_y;
} Desktop;

void Desktop_ProcessMouse(
	Desktop* desktop,
	uint16_t mouse_x,
	uint16_t mouse_y,
	uint8_t mouse_buttons);

Desktop* Desktop_Create(
	Rendering_Context* context);

Window* Desktop_Create_Window(
	Desktop* desktop,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height);

void Desktop_Paint(
	Desktop* desktop);

list_t* Desktop_Get_Windows_Above(
	Desktop* desktop,
	Window* window);

#endif

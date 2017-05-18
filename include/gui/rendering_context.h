#ifndef _RENDERING_CONTEXT_H
#define _RENDERING_CONTEXT_H

#include <lib/stdint.h>

#include <util/list.h>

#include <gui/rect.h>

typedef struct _Rendering_Context
{
	uint32_t* buffer;

	uint32_t width;
	uint32_t height;

	list_t* clip_rects;
} Rendering_Context;

Rendering_Context* Rendering_Context_New(
	uint32_t width,
	uint32_t height,
	uint32_t* buffer);

void Rendering_Context_Subtract_Clip_Rect(
	Rendering_Context* context,
	Rect* subtracted_rect);

void Rendering_Context_Add_Clip_Rect(
	Rendering_Context* context,
	Rect* added_rect);

void Rendering_Context_Clear_Clip_Rects(
	Rendering_Context* context);

void Rendering_Context_Clipped_Rect(
	Rendering_Context* context,
	int32_t x,
	int32_t y,
	uint32_t width,
	uint32_t height,
	Rect* clip_area,
	uint32_t color);

void Rendering_Context_Fill_Rect(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	uint32_t color);

void Rendering_Context_Draw_Rect(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height,
	uint32_t color);

void Rendering_Context_Draw_Horizontal_Line(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t length,
	uint32_t color);

void Rendering_Context_Draw_Vertical_Line(
	Rendering_Context* context,
	uint32_t x,
	uint32_t y,
	uint32_t length,
	uint32_t color);

#endif

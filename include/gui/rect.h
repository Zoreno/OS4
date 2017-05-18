#ifndef _RECT_H
#define _RECT_H

#include <util/list.h>

typedef struct _Rect
{
	int top;
	int left;
	int bottom;
	int right;
} Rect;

Rect* Rect_New(
	int top,
	int left,
	int bottom,
	int right);

list_t* Rect_Split(
	Rect* subject_rect,
	Rect* cutting_rect);

#endif

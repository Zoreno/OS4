#ifndef _MOUSE_H
#define _MOUSE_H

#include <lib/stdint.h>

/**
 *	Enumeration of the currently handled mouse buttons.
 */
typedef enum
{
	/**
	 * Left mouse button
	 */
	MOUSE_BUTTON_LEFT 	= 1,

	/**
	 * Middle mouse button
	 */
	MOUSE_BUTTON_MIDDLE = 2,

	/**
	 * Right mouse button
	 */
	MOUSE_BUTTON_RIGHT = 4,

	/**
	 * Mouse button 4
	 */
	MOUSE_BUTTON_4 = 8,

	/**
	 * Mouse button 5
	 */
	MOUSE_BUTTON_5 = 0x10,
} mouse_button_t;

/**
 * Mouse Type Enumeration.
 */
typedef enum
{
	/**
	 * Normal mouse without wheel.
	 */
	MOUSE_NORMAL,

	/**
	 * Mouse with mousewheel
	 */
	MOUSE_WHEEL,

	/**
	 * Mouse with support for two-dimensional mouse movement
	 * and 5 buttons.
	 */
	MOUSE_WHEEL_BUTTON
	
} mouse_type_t;
	
void mouse_install();

void mouse_set_sample_freq(uint8_t freq);

void mouse_set_position(int32_t x, int32_t y);
void mouse_get_position(int32_t* x, int32_t* y);

int mouse_is_pressed(mouse_button_t button);

// Mouse Events


// Mouse Moved Event

typedef struct _mouse_moved_event
{
	int32_t x;
	int32_t y;

	int32_t dx;
	int32_t dy;
} mouse_moved_event_t;

typedef void(*mouse_moved_event_handler)(mouse_moved_event_t*);

void register_mouse_moved_handler(mouse_moved_event_handler newHandler);

// Mouse Button Event

typedef enum
{
	MOUSE_ACTION_PRESS = 0,
	MOUSE_ACTION_RELEASE = 1
} mouse_button_action_t;

typedef struct _mouse_button_event
{
	uint32_t button;
	mouse_button_action_t action;
} mouse_button_event_t;

typedef void(*mouse_button_event_handler)(mouse_button_event_t*);

void register_mouse_button_handler(mouse_button_event_handler newHandler);

// Mouse Scroll Event

typedef struct _mouse_scroll_event
{
	int32_t horizontal;
	int32_t vertical;
} mouse_scroll_event_t;

typedef void(*mouse_scroll_event_handler)(mouse_scroll_event_t*);

void register_mouse_scroll_handler(mouse_scroll_event_handler newHandler);


#endif



#include <lib/stdio.h>

KEYCODE	getch () {

	KEYCODE key = KEY_UNKNOWN;

	//! wait for a keypress
	while (key==KEY_UNKNOWN)
		key = keyboard_get_last_key ();

	//! discard last keypress (we handled it) and return
	keyboard_discard_last_key ();

	return key;
}
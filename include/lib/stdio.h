#ifndef _STDIO_H
#define _STDIO_H

#include <lib/stdint.h>

#include <input/keyboard.h>
#include <serial/serial.h>

int printf(const char* format, ...);
int putch(char c);
int puts(const char* s);

/**
* @brief Prints a character to a COM port.
*
* @param port	Port to write to.
* @param c		ASCII Character to write.
*/
void serial_putch(COM_port port, const char c);

/**
* @brief Prints a C-string to a COM port.
*
* @param port	Port to write to.
* @param str	C-string to write.
*/
void serial_puts(COM_port port, const char* str);

/**
* @brief Prints a formatted string to a COM port.
*
* @param port	Port to write to.
* @param format	Format string.
* @param ...	Additional parameters.
*/
int serial_printf(COM_port port, const char* format, ...);


KEYCODE	getch ();

void clearScreen();
void setCursor(uint32_t x, uint32_t y);
void getCursor(uint32_t* x, uint32_t* y);

#endif
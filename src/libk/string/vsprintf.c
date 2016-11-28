#include <lib/string.h>

#include <lib/stdarg.h>
#include <lib/size_t.h>
#include <lib/ctype.h>

/*
	Test Usage:

	vsprintf("%i", 4); // "4"
	vsprintf("%*i", 4, 4); // "   4"
	vsprintf("%*i", 4, 12345); // "12345"
	vsprintf("%(4)i", 4); // "   4" 
	vsprintf("%(4)i", 12345); // "12345" 

*/

// (Not as in C std lib specification. This is a subset though)
// Formal Grammar rule: 
// format:  (%(flags)(width)specifier | character)*

/*
 flags:

 -: 		Left-justify
 +: 		Force sign
 (space):	Insert space if no sign
 #:			Print prefix 
 0:			Left-pad with 0 instead of space
*/

/*
 width:

 (number):	Minimum number of characters to print
 *:			Minimum number of characters to print as argument before number.

*/

/*
 specifier:

 c:			Character
 d | i:		Signed dec
 o:			Signed octal
 s:			String
 u:			Unsigned dec
 x | X:		Unsigned hex			
 p:			Pointer address
 %:			'%' Character
*/ 

typedef struct {

	uint32_t width;

	char specifier;

	uint8_t left_justify : 1;
	uint8_t force_sign : 1;
	uint8_t space : 1;
	uint8_t print_prefix : 1;
	uint8_t zero_pad : 1;
	uint8_t width_as_arg : 1;
	uint8_t reserved : 2;
} flags_t;

int is_flag(char c);
int is_specifier(char c);

int parseCommand(const char* format, flags_t* flags, uint32_t* length);

static char _flags[] = "-+ #0";

int is_flag(char c){
	for(size_t p = 0; p < strlen(_flags);++p){
		if(_flags[p] == c){
			return 1;
		}
	}
	return 0;
}

static char _specifiers[] = "cdiosuxXp%";

int is_specifier(char c){

	for(size_t p = 0; p < strlen(_specifiers);++p){
		if(_specifiers[p] == c){
			return 1;
		}
	}
	return 0;
}

int parseCommand(const char* format, flags_t* flags, uint32_t* length){

	char* currentChar = format;

	// First parse all non-specifier chars

	// Check all flags
	while(is_flag(*currentChar)){
		switch(*currentChar){
			case '-':{ 
				flags->left_justify = 1;
				break;
			}
			case '+':{
				flags->force_sign = 1;
				break;
			}
			case ' ':{
				flags->space = 1;
				break;
			}
			case '#':{
				flags->print_prefix = 1;
				break;
			}
			case '0':{
				flags->zero_pad = 1;
				break;
			}
		}
		// Advance to the next char
		++currentChar;
		++(*length);
	}

	// Check width arg(if any)
	if(*currentChar == '*'){
		flags->width_as_arg = 1;
		// Advance to the next char
		++currentChar;
		++(*length);
	} else if (*currentChar == '('){
		// Advance
		++currentChar;

		// Allocate a buffer for the number to be stored
		char buf[32] = {0};
		size_t i = 0;

		while(isdigit(*currentChar)){
			buf[i++] = *(currentChar++);
		} 

		// If the next character isn't a ')', the format is wrong. Return error.
		if(*currentChar != ')'){
			return 1;
		}
		++currentChar;
		buf[i] = '\0'; // Make sure to null terminate

		flags->width = atoi(buf);

		(*length) += strlen(buf) + 2; // Length of buffer + '(' and ')'
	}

	// Then get the specifier

	if(is_specifier(*currentChar)){
		flags->specifier = *currentChar;
		++(*length);
		return 0;
	}

	// Specifier is mandatory, return error
	return 1;
}

#define output_char(c) (str[loc++] = c)
#define output_string(s) 	{strcpy(&str[loc], s); \
							loc += strlen(s);}
#define PAD(width_delta, pad_char) 							\
		{for(int i_pad = 0; i_pad < width_delta; ++i_pad) 	\
		{ 													\
			output_char(pad_char); 							\
		}}

int vsprintf(char* str, const char* format, va_list ap){

	// Sanity check
	if(!str)
		return 0;

	if(!format)
		return 0;

	// Position in output buffer
	size_t loc = 0;

	// Position in format string
	size_t i = 0;

	// Length of the format string
	const size_t format_length = strlen(format);

	// Loop until all characters in format buffer is parsed
	while(i < format_length){
		char c = format[i++];
		
		// Check if char is beginning of command
		if(c == '%'){
			// Initiate flags to 0(No flags are set as default)
			flags_t flags;
			memset(&flags, 0, sizeof(flags_t));

			// The length of the command in format buffer
			uint32_t length = 0;

			// &format[i] gives the string beginning at the char after '%'
			if(parseCommand(&format[i], &flags, &length)){
				// We have an error in the format
				return 0;
			}

			// Adjust for command length
			i += length;

			// Calculate minimum width
			int width = 0;
			if(flags.width != 0){
				width = flags.width;
			} else if(flags.width_as_arg){
				width = va_arg(ap, int);
			}

			char pad_char = (flags.zero_pad) ? '0' : ' ';

			switch(flags.specifier){
				case 'd':
				case 'i':{
					// Fetch integer from stack
					int c = va_arg(ap, int);

					// Allocate a small buffer
					char s[32] = {0};

					// Convert int to string
					itoa_s(c, 10, s);

					// Calculate pad length
					int width_delta = width - strlen(s);

					// If int is pos or 0
					if(c >= 0){

						// Manually insert '+' if positive
						if(flags.force_sign){
							output_char('+');
							width_delta -= 1;

						// Insert ' ' so positive values will line up with 
						// negative values
						} else if(flags.space){
							output_char(' ');
							width_delta -= 1;
						}
					}

					if(flags.left_justify){
						output_string(s);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_string(s);
					}
					
					break;
				}

				case 'o':{
					// Fetch integer from stack
					int c = va_arg(ap, int);

					// Allocate a small buffer
					char s[32] = {0};

					
					// Convert int to string
					itoa_s(c, 8, s);
					

				
					// Calculate pad length
					int width_delta = width - strlen(s);

					// If int is pos or 0
					if(c >= 0){
						
						// Manually insert '+' if positive
						if(flags.force_sign){
							output_char('+');
							width_delta -= 1;

						// Insert ' ' so positive values will line up with 
						// negative values
						} else if(flags.space){
							output_char(' ');
							width_delta -= 1;
						}
					}

					if(flags.print_prefix){
						output_string("0");
						width_delta -= 1;
					}

					if(flags.left_justify){
						output_string(s);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_string(s);
					}
					
					break;
				}

				case 's':{
					// Fetch string pointer from stack
					char* c = (char*) va_arg(ap, char*);

					// Allocate a small buffer
					// TODO: Increase if needed
					char s[32]={0};

					// Copy string to buffer
					strcpy(s, (const char*)c);

					// Calculate pad length
					int width_delta = width - strlen(s);


					if(flags.left_justify){
						output_string(s);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_string(s);
					}
					
					break;
				}

				case 'u':{
					// Fetch integer from stack
					unsigned int c = va_arg(ap, unsigned int);

					// Allocate a small buffer
					char s[32] = {0};

					// Convert int to string
					itoa(c, 10, s);

					// Calculate pad length
					int width_delta = width - strlen(s);


					if(flags.left_justify){
						output_string(s);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_string(s);
					}
					
					break;
				}

				case 'p': // A minor hack
				case 'x':
				case 'X':{
					// Fetch integer from stack
					unsigned int c = va_arg(ap, unsigned int);

					// Allocate a small buffer
					char s[32] = {0};

					itoa(c, 16, s);

					// Calculate pad length
					int width_delta = width - strlen(s);

					if(flags.print_prefix){
						output_string("0x");
						width_delta -= 2;
					}

					if(flags.left_justify){
						output_string(s);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_string(s);
					}
					
					break;
				}

				case 'c':{

					char c = va_arg(ap, char);

					int width_delta = width - 1;
					
					if(flags.left_justify){
						output_char(c);
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_char(c);
					}
					
					break;
				}

				case '%':{

					int width_delta = width - 1;
					
					if(flags.left_justify){
						output_char('%');
						PAD(width_delta, pad_char);
					} else {
						PAD(width_delta, pad_char);
						output_char('%');
					}
					
					break;
				}

			}


		} else {
			output_char(c);
		}
	}
}
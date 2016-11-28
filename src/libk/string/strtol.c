#include <lib/string.h>

long strtol(const char* nptr, char** endptr, int base){
	const char* s = nptr; // Pointer to next char to process
	unsigned long acc; // Accumulator for the value
	int c; // Currentrly processed char.
	unsigned long cutoff; 	// Largest possible number that fit into a long in
							// a given base divided by the base.
	int neg = 0; // Indicates whether the string represents a negative number
	int any;
	int cutlim; // Last digit of cutoff value

	// Skip whitespace
	do{

		c = *s++;

	} while (isspace(c));

	// Check the sign (if any) passed in the string.
	if(c == '-'){

		neg = 1;
		c = *s++;

	} else if(c == '+') {

		c = *s++;
	}

	// If the base is either unspecified(0) or 16, allow "0x" or "0X" to
	// precede number. This will set base to 16 if it were unspecified.
	if((base == 0 || base == 16) && c == '0' && (*s == 'x'|| *s == 'X')){

		c = s[1];
		s += 2;
		base = 16;
	} 

	// If the base is either unspecified(0) or 2, allow "0b" or "0B" to
	// precede number. This will set base to 2 if it were unspecified.
	else if ((base == 0 || base == 2) && c == '0' && (*s == 'b'|| *s == 'B')){

		c = s[1];
		s += 2;
		base = 2;
	}

	// If the base still is unspecified, assume octal if number starts with a 0,
	// else decimal
	else if(base == 0){

		if(c == '0'){

			base = 8;
		} else {

			base = 10;
		}
	}

	// Calculate cutoff and offset
	cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
	cutlim = cutoff % (unsigned long)base;
	cutoff /= (unsigned long)base;

	for(acc = 0, any = 0;; c = *s++){

		// Find numerical value of given char
		if(isdigit(c)){

			c -= '0';
		} else if(isalpha(c)){

			if(isupper(c)){

				c -= 'A' - 10;
			} else {

				c -= 'a' - 10;
			}
		} else {

			break; // Char is not number or alphabetical char.
		}

		if(c >= base){

			break; // Digit is larger than base
		}

		// Check if appending the next character will result in an overflow.
		if(any < 0 || acc > cutoff || acc == cutoff && c > cutlim){
			
			any = -1;
		} else {

			any = 1;
			acc *= base;
			acc += c;
		}
	}

	// If overflow occured, set accumulator to maximum value 
	if(any < 0){

		acc = neg ? LONG_MIN : LONG_MAX;
	} else if(neg){

		acc = -acc;
	}

	// If endptr point to a valid address, store terminating character there.
	if(endptr != 0){

		*endptr = (char*)(any ? (s - 1) : nptr);
	}

	// Return the calculated value
	return (acc);
}

unsigned long strtoul(const char* nptr, char** endptr, int base){
	const char* s = nptr; // Pointer to next char to process
	unsigned long acc; // Accumulator for the value
	int c; // Currentrly processed char.
	unsigned long cutoff; 	// Largest possible number that fit into a long in
							// a given base divided by the base.
	int neg = 0; // Indicates whether the string represents a negative number
	int any;
	int cutlim; // Last digit of cutoff value

	// Skip whitespace
	do{

		c = *s++;

	} while (isspace(c));

	// Check the sign (if any) passed in the string.
	if(c == '-'){

		neg = 1;
		c = *s++;

	} else if(c == '+') {

		c = *s++;
	}

	// If the base is either unspecified(0) or 16, allow "0x" or "0X" to
	// precede number. This will set base to 16 if it were unspecified.
	if((base == 0 || base == 16) && c == '0' && (*s == 'x'|| *s == 'X')){

		c = s[1];
		s += 2;
		base = 16;
	} 

	// If the base is either unspecified(0) or 2, allow "0b" or "0B" to
	// precede number. This will set base to 2 if it were unspecified.
	else if ((base == 0 || base == 2) && c == '0' && (*s == 'b'|| *s == 'B')){

		c = s[1];
		s += 2;
		base = 2;
	}

	// If the base still is unspecified, assume octal if number starts with a 0,
	// else decimal
	else if(base == 0){

		if(c == '0'){

			base = 8;
		} else {

			base = 10;
		}
	}

	// Calculate cutoff and offset
	cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
	cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;

	for(acc = 0, any = 0;; c = *s++){

		// Find numerical value of given char
		if(isdigit(c)){

			c -= '0';
		} else if(isalpha(c)){

			if(isupper(c)){

				c -= 'A' - 10;
			} else {

				c -= 'a' - 10;
			}
		} else {

			break; // Char is not number or alphabetical char.
		}

		if(c >= base){

			break; // Digit is larger than base
		}

		// Check if appending the next character will result in an overflow.
		if(any < 0 || acc > cutoff || acc == cutoff && c > cutlim){
			
			any = -1;
		} else {

			any = 1;
			acc *= base;
			acc += c;
		}
	}

	// If overflow occured, set accumulator to maximum value 
	if(any < 0){

		acc = ULONG_MAX;
	} else if(neg){

		acc = -acc;
	}

	// If endptr point to a valid address, store terminating character there.
	if(endptr != 0){

		*endptr = (char*)(any ? (s - 1) : nptr);
	}

	// Return the calculated value
	return (acc);
}
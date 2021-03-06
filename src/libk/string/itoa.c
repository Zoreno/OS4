#include <lib/string.h>

char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

void itoa(uint32_t i, uint32_t base, char* buf){
	int pos = 0;
	int opos = 0;
	int top = 0;

	char tbuf[32] = {0};

	// Sanity check (we do only handle base up to 16)
	if(i == 0 || base > 16){
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while(i != 0){
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}

	top = pos--;

	for(opos = 0; opos<top; pos--, opos++){
		buf[opos] = tbuf[pos];
	}

	buf[opos] = 0;

}
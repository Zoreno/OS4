#include <lib/string.h>

void itoa_s(int32_t i, uint32_t base, char* buf){
	if(base > 16)
		return;

	if(i < 0){
		*buf++ = '-';
		i*=-1;
	}
	itoa(i, base, buf);
}
#include <sync/mutex.h>

void lock(mutex_t* m)
{
	asm volatile (	"mov			$1, %%al		;"
				  	"try_lock:						;"
					"xchg			%%al, (%%edi)	;"
					"test			%%al, %%al		;"
					"jz				exit			;"
					"hlt							;"
					"jmp			try_lock		;"
					"exit:							;"
					:: "D"(m));
}

void unlock(mutex_t* m)
{
	asm volatile ("movb $0, (%%edi)" :: "D" (m));
}

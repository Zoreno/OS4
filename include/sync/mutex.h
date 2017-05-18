#ifndef _MUTEX_H
#define _MUTEX_H

typedef unsigned char mutex_t;

void lock(mutex_t* m);
void unlock(mutex_t* m);

#endif

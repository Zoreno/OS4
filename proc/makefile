
CC = ~/opt/cross/bin/i686-elf-gcc
AR = ~/opt/cross/bin/i686-elf-ar

CCFLAGS = -ffreestanding -nostdlib -m32 -nostartfiles

CCEXECFLAGS = $(CCFLAGS)
CCLIBFLAGS = $(CCFLAGS)

all: lib-link compile
	$(CC) -L . -o proc.elf main.o -lc

compile:
	$(CC) $(CCEXECFLAGS) -I . -c main.c

lib:
	$(CC) -c $(CCLIBFLAGS) test.c

lib-link: lib lib-crt lib-main
	$(AR) rs libc.a test.o crt0.o libmain.o

lib-crt:
	$(CC) -c $(CCLIBFLAGS) crt0.S

lib-main:
	$(CC) -c $(CCLIBFLAGS) libmain.c

clean:
	rm -f *.o *.a *.elf





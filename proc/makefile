
CC = ~/opt/cross/bin/i686-elf-gcc
AR = ~/opt/cross/bin/i686-elf-ar

CCFLAGS = -ffreestanding -nostdlib -m32 -nostartfiles

INCLUDE_PATH = -I ../libc/include
LIB_PATH = -L ../libc/lib

all: compile
	$(CC) $(CCFLAGS) $(LIB_PATH) -T link.ld -o proc.elf main.o -lc

compile:
	$(CC) $(CCFLAGS) $(INCLUDE_PATH) -c main.c





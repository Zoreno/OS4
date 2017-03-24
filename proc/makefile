
CC = ~/opt/cross/bin/i686-elf-gcc
AR = ~/opt/cross/bin/i686-elf-ar

CCFLAGS = -ffreestanding -nostdlib -m32 -nostartfiles

INCLUDE_PATH = ../libc/include
LIB_PATH = ../libc/lib

all: compile
	$(CC) $(CCFLAGS) -L $(LIB_PATH) -o proc.elf main.o -lc

compile:
	$(CC) $(CCFLAGS) -I $(INCLUDE_PATH) -c main.c





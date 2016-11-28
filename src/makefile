SUBDIRS = libk kernel hal mm input floppy serial pci ata vbe acpi cmos

OBJECTS =

CC = gcc
CFLAGS=-g3 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -I $(INC_DIR)
ASFLAGS=-felf32

export CC
export CFLAGS
export ASFLAGS

all: subdirs $(OBJECTS)

clean:
	-rm *.o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(KERNEL_OBJ_DIR)/$@ $<

.s.o:
	nasm $(ASFLAGS) -o $(KERNEL_OBJ_DIR)/$@ $<

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
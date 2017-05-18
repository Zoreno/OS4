SUBDIRS = 

OBJECTS = list.o

CC = $(CC_DIR)/i686-elf-gcc
CFLAGS=-g -m32 -nostdlib -nostdinc -fverbose-asm -fno-builtin -fno-stack-protector -I $(INC_DIR)
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

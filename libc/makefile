SUBDIRS = src

LIBC_CC = ~/opt/cross/bin/i686-elf-gcc
LIBC_AR = ~/opt/cross/bin/i686-elf-ar

LIBC_ROOT_DIR := $(shell pwd)
LIBC_INC_DIR = $(LIBC_ROOT_DIR)/include
LIBC_INTERMEDIATE_DIR = $(LIBC_ROOT_DIR)/intermediate
LIBC_OUTPUT_DIR = $(LIBC_ROOT_DIR)/lib

LIBC_CCFLAGS = -c -ffreestanding -nostdlib -m32 -nostartfiles -I $(LIBC_INC_DIR)
LIBC_LINKFLAGS = -ffreestanding -nostdlib -m32 -nostartfiles -I $(LIBC_INC_DIR)
LIBC_ASFLAGS = -felf32

LIBC_TARGET_NAME = libc.a

export LIBC_ALL_OBJECTS
export LIBC_CC
export LIBC_AR
export LIBC_ROOT_DIR
export LIBC_INC_DIR
export LIBC_INTERMEDIATE_DIR
export LIBC_OUTPUT_DIR
export LIBC_CCFLAGS
export LIBC_ASFLAGS

all: link

link: subdirs
	$(LIBC_AR) rc $(LIBC_OUTPUT_DIR)/$(LIBC_TARGET_NAME) $(wildcard $(LIBC_INTERMEDIATE_DIR)/*.o)
	
clean:
	rm $(LIBC_INTERMEDIATE_DIR)/*.o $(LIBC_OUTPUT_DIR)/$(LIBC_TARGET_NAME)

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

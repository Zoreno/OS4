SUBDIRS = src

ROOT_DIR := $(shell pwd)
INC_DIR = $(ROOT_DIR)/include
KERNEL_OBJ_DIR = $(ROOT_DIR)/objects/kernel
KERNEL_OUTPUT_DIR = $(ROOT_DIR)/output
KERNEL_DISASM_DIR = $(ROOT_DIR)/disassembly

export ALL_OBJECTS
export ROOT_DIR
export INC_DIR
export KERNEL_OBJ_DIR
export KERNEL_OUTPUT_DIR
export KERNEL_DISASM_DIR

LDFLAGS=-T $(ROOT_DIR)/src/link.ld -m elf_i386 -s -g

all: link
	bash mk_iso.sh
	@echo Compilation Finished

run: all
	bash run_bochs.sh

clean:
	-rm $(KERNEL_OBJ_DIR)/*.o
	-rm $(KERNEL_OUTPUT_DIR)/kernel.elf

link: subdirs
	ld $(LDFLAGS) -o $(KERNEL_OUTPUT_DIR)/kernel.elf $(wildcard $(KERNEL_OBJ_DIR)/*.o)
	@echo Generating disassembly...
	objdump -S  $(KERNEL_OUTPUT_DIR)/kernel.elf > $(KERNEL_DISASM_DIR)/kernel_full.txt

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
SUBDIRS = 
OBJECTS = memchr.o memcmp.o memcpy.o memmove.o memset.o strlen.o  strcpy.o strncpy.o

all: subdirs $(OBJECTS)

%.o: %.c
	$(LIBC_CC) $(LIBC_CCFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.s.o:
	nasm $(LIBC_ASFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

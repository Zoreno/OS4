OBJECTS = \
strlen.o \
strcpy.o \
strcmp.o \
strcat.o \
strchr.o \
strcicmp.o \
strncmp.o \
strncicmp.o \
strncpy.o \
strncat.o \
memcmp.o \
memcpy.o \
memmove.o \
memset.o \
memsetl.o \
itoa.o \
itoa_s.o \
vsprintf.o \
atoi.o \
strtol.o \
kmalloc.o 

SUBDIRS =

all: subdirs $(OBJECTS) 

clean:
	-rm *.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $(KERNEL_OBJ_DIR)/$@ $<

.s.o:
	nasm $(ASFLAGS) -o $(KERNEL_OBJ_DIR)/$@ $<

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@
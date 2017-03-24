OBJECTS = \
loader.o \
main.o \
a20.o \
monitor.o \
panic.o \
exception.o \
int32.o \
syscall.o

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

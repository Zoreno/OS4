OBJECTS = \
rendering_context.o \
window.o \
desktop.o \
rect.o

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

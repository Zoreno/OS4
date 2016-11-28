OBJECTS = \
putch.o \
puts.o \
printf.o \
clearScreen.o \
setCursor.o \
getCursor.o \
getch.o \
serial_putch.o \
serial_puts.o \
serial_printf.o

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
SUBDIRS = stdio string
OBJECTS = crt0.o crt1.o crti.o crtn.o libmain.o

all: subdirs $(OBJECTS)

%.o: %.c
	$(LIBC_CC) $(LIBC_CCFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.s.o:
	nasm $(LIBC_ASFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

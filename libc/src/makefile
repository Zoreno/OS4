SUBDIRS = 
OBJECTS = crt0.o libmain.o putch.o puts.o

all: subdirs $(OBJECTS)

%.o: %.c
	$(LIBC_CC) $(LIBC_CCFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.s.o:
	$(LIBC_CC) $(LIBC_CCFLAGS) -o $(LIBC_INTERMEDIATE_DIR)/$@ $<

.PHONY: subdirs $(SUBDIRS)

subdirs: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

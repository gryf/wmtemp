CC=gcc -O0
AR=ar
RANLIB=ranlib

system=Linux
HAVE_NVCONTROL=no
OBJECTS=backend.o bios.o config.o error.o nv30.o nv40.o nv50.o info.o overclock.o utils.o i2c.o xf86i2c.o adt7473.o f75375.o lm99.o w83781d.o w83l785r.o libc_wrapper.o
CFLAGS= -I../.. -I../nvcontrol

ifeq ($(system), FreeBSD)
    OBJECTS+=back_bsd.o
else
    ifeq ($(system), Win32)
	OBJECTS+=back_win32.o
    else
	OBJECTS+=back_linux.o
    endif
endif

ifeq ($(HAVE_NVCONTROL), yes)
    OBJECTS+=nvcontrol.o
endif

.PHONY: clean distclean
all: libbackend.a


libbackend.a: $(OBJECTS)
	$(AR) cru libbackend.a $(OBJECTS)
	$(RANLIB) libbackend.a

clean:
	rm -f *.o *.a

distclean: clean
	rm -f Makefile

install:
uninstall:
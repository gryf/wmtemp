# By RedSeb 1999, Liverbugg 2002

SRC = src/wmtemp.c

EXE = src/wmtemp

OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -O3

LIB = -L/usr/X11R6/lib -lXpm -lXext -lX11 -ldockapp

INSTALL = /usr/bin/install

CC = gcc

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJ) $(LIB)
	strip $(EXE)

$(OBJ): %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXE) src/*.o

install:
	$(INSTALL) $(EXE) /usr/local/bin/

uninstall:
	rm -rf /usr/local/bin/$(EXE)

remove: uninstall

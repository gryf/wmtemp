# By RedSeb 1999, Liverbugg 2002


SRC = wmtempnv.c ../wmgeneral/wmgeneral.c ../wmgeneral/misc.c ../wmgeneral/list.c

EXE = wmtempnv

OBJ = $(SRC:.c=.o)

CFLAGS = -Wall -O3

LIB = -L/usr/X11R6/lib -lXpm -lXext -lX11 -lsensors

INSTALL = /usr/bin/install

CC = gcc

all: $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIB)
	strip $(EXE)

$(OBJ): %.o : %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXE)
	rm -rf *.o
	rm -rf ../wmgeneral/*.o

install:
	$(INSTALL) $(EXE) /usr/local/bin/
uninstall:
	rm -rf /usr/local/bin/$(EXE)

remove: uninstall

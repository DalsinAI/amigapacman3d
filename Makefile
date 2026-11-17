# Makefile for pacman

CC = gcc

OBJ = pacman.o pac_ext.o
BIN = pacman

INCS = 
LIBS = 

PCFLAGS = 
CFLAGS  = $(INCS) -W -Wall
LDFLAGS = $(LIBS) `allegro-config --libs --static`

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

pacman.o: pacman.c pac.h data/pacdata.h data/enem.h
	$(CC) $(PCFLAGS) $(CFLAGS) -c pacman.c -o pacman.o

pac_ext.o: pac_ext.c data/enem.h
	$(CC) $(PCFLAGS) $(CFLAGS) -c pac_ext.c -o pac_ext.o

.PHONY: clean
clean: 
	-rm $(BIN) $(OBJ)


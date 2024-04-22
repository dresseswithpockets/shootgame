ifeq ($(OS),Windows_NT)
	BINEXT := .exe
	LIBS=-lraylib -lwinmm -lgdi32
endif

OUTDIR=bin
SRCDIR=src
IDIR=include
LDIR=lib
ODIR=obj

CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR) -Wall

_DEPS = raylib.h
DEPS=$(patsubst %,$(IDIR)/%,$(_DEPS)) src/entity.h src/game.h

_OBJ = main.o game.o entity.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)/main.exe: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(OUTDIR)/*

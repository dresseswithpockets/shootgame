ifeq ($(OS),Windows_NT)
	BINEXT := .exe
	LIBS=-lraylib -lwinmm -lgdi32
	LDIR=lib/windows
endif

OUTDIR=bin
SRCDIR=src
IDIR=include
ODIR=obj

CC=gcc
CFLAGS=-I$(IDIR) -L$(LDIR) -Wall -Werror

PCH_SRC = $(SRCDIR)/pch.h
PCH_DEP = $(SRCDIR)/pch-src.h
PCH_HEADERS = $(IDIR)/raylib.h $(IDIR)/raymath.h $(IDIR)/rlgl.h
PCH_OUT = $(PCH_SRC).gch

_SRC_HEADERS = entity.h game.h
SRC_HEADERS=$(SRCDIR)/entity.h $(SRCDIR)/game.h

_OBJ = main.o game.o entity.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

# main output
$(OUTDIR)/main$(BINEXT): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

# precompiled header
$(PCH_OUT): $(PCH_SRC) $(PCH_DEP) $(PCH_HEADERS)
	$(CC) $(CFLAGS) -o $@ $<

# translation units
$(ODIR)/%.o: $(SRCDIR)/%.c $(SRC_HEADERS) $(PCH_OUT)
	$(CC) $(CFLAGS) -include $(PCH_SRC) -c -o $@ $<

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o
	rm -f $(OUTDIR)/*
	rm -f $(PCH_OUT)

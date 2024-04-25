ifndef BUILD_DEBUG
	BUILD_DEBUG=1
endif

CC=gcc
CFLAGS=-std=c2x -Wall -Werror -DPLATFORM_DEKSTOP
# separated out linker flags, since these shouldnt be used when compiling the PCH
LFLAGS=$(_EMPTY_)

ifeq ($(OS),Windows_NT)
	BINEXT := .exe
	LIBS=-lraylib -lwinmm -lgdi32 -static
	CFLAGS+=-Llib/windows
else
	BINEXT := .x86_64
	LIBS=-lraylib -lGL -lm -lpthread -ldl -lrt -lX11
	CFLAGS+=-Llib/linux
endif

OUTDIR=bin
SRCDIR=src
IDIR=include
ODIR=obj
OUTBIN=$(OUTDIR)/game$(BINEXT)

# for publishing
PUBDIR=publish
PUBBIN=$(PUBDIR)/game$(BINEXT)
ASSETDIR=assets

CFLAGS+=-I$(IDIR)

ifeq ($(BUILD_RELEASE),1)
	ifdef BUILD_WEB
		CFLAGS+=-Os
	else
		CFLAGS+=-O2
	endif
endif

ifeq ($(BUILD_DEBUG),1)
	CFLAGS+=-g -fsanitize=undefined -fsanitize-trap
else
	ifeq ($(OS),Windows_NT)
		CFLAGS+=-DSG_USE_WINMAIN
		LFLAGS+=-Wl,--subsystem,windows
	endif
endif

PCH_SRC = $(SRCDIR)/pch.h
PCH_DEP = $(SRCDIR)/pch-src.h
PCH_HEADERS = $(IDIR)/raylib.h $(IDIR)/raymath.h $(IDIR)/rlgl.h
PCH_OUT = $(PCH_SRC).gch

_SRC_HEADERS = entity.h game.h math.h input.h genarray.h sprite.h dynamic_array.h
SRC_HEADERS=$(patsubst %,$(SRCDIR)/%,$(_SRC_HEADERS))

_OBJ = main.o game.o entity.o input.o sprite.o dynamic_array.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

# main output
$(OUTBIN): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LFLAGS) $(LIBS)

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
	rm -rf $(PUBDIR)

.PHONY: publish

publish: $(OUTBIN)
	mkdir -p publish
	cp $(OUTBIN) $(PUBBIN)
	cp $(ASSETDIR) $(PUBDIR)/$(ASSETDIR) -r

IDIR = ./include
CC = g++
CFLAGS = -I$(IDIR)

ODIR = ./obj
LDIR = ./lib
SDIR = ./src

LIBS = -lm

_DEPS = http_tcp_server_linux.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = http_tcp_server_linux.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

_SRC = http_tcp_server_linux.cpp
SRC = $(patsubst %,$(SDIR)/%,$(_SRC))

MAIN = src/main.cpp

$(ODIR)/%.o: $(SRC) $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

out/mcpy: $(OBJ) $(MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o out/mcpy
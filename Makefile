.PHONY: all run build clean

CC=cc
CFLAGS=-Wall -Wextra -pedantic -ggdb
EXEC=build/hangc

all: build

run: build
	./$(EXEC)

build:
	mkdir -p build
	$(CC) -o $(EXEC) src/hangc.c -Iinc $(CFLAGS)

clean:
	rm -rf build


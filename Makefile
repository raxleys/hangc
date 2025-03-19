.PHONY: all run build check clean

CC=cc
EXEC=build/hangc

all: build

run: build
	./$(EXEC)

check: build
	valgrind --leak-check=full ./$(EXEC)

build:
	mkdir -p build
	$(CC) -o $(EXEC) src/main.c -Iinc -ggdb
	chmod +x $(EXEC)

clean:
	rm -rf build


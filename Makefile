.PHONY: run all clean

run: all
	./build/hangc

all:
	mkdir -p build
	gcc -o build/hangc src/main.c -Iinc -ggdb
	chmod +x build/hangc

clean:
	rm -rf build


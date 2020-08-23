CC = gcc

CFLAGS = -Wall -Wextra -Wpedantic -g -Og

.PHONY: all clean

all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(CFLAGS) -o test_mtojson test_mtojson.o mtojson.o

clean:
	rm -f mtojson.o test_mtojson.o test_mtojson

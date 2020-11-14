CC = gcc

CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -g -Og -fno-common -Wshadow
ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer

.PHONY: all clean

all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(CFLAGS) $(ASAN) -o test_mtojson test_mtojson.o mtojson.o

clean:
	rm -f mtojson.o test_mtojson.o test_mtojson

cppcheck:
	cppcheck --suppress=missingIncludeSystem -I. --template gcc --enable=all --check-config *.[ch]

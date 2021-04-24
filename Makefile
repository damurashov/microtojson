CC = gcc

CFLAGS = -std=c99
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -fno-common -Wshadow
CFLAGS += -g -Og

ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS += $(ASAN)

.PHONY: all clean

all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(CFLAGS) -o test_mtojson test_mtojson.o mtojson.o

clean:
	rm -f mtojson.o test_mtojson.o test_mtojson

cppcheck:
	cppcheck --suppress=missingIncludeSystem -I. --template gcc --enable=all --check-config *.[ch]

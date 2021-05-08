CC=gcc

CFLAGS = -std=c99
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -Wformat=2 -Wduplicated-cond -Wjump-misses-init -Wnull-dereference
CFLAGS += -fno-common -Wshadow -Wconversion -Wmissing-declarations
CFLAGS += -g -O2

ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS += $(ASAN)

ifndef ASAN
WSTACK = -Wstack-usage=64 -fstack-usage
endif

.PHONY: all clean

all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h
	$(CC) $(CFLAGS) $(WSTACK) -c -o mtojson.o mtojson.c

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(CFLAGS) -o test_mtojson test_mtojson.o mtojson.o

clean:
	rm -f mtojson.o test_mtojson.o test_mtojson mtojson.su

cppcheck:
	cppcheck --suppress=missingIncludeSystem -I. --template gcc --enable=all --check-config *.[ch]

CC=gcc

CFLAGS += -std=c99
CFLAGS += -g
CFLAGS += -O2
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpedantic

CFLAGS += -Wconversion
CFLAGS += -Wduplicated-cond
CFLAGS += -Wformat=2
CFLAGS += -Wjump-misses-init
CFLAGS += -Wmissing-declarations
CFLAGS += -Wnull-dereference
CFLAGS += -Wshadow

CFLAGS += -fno-common

ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS += $(ASAN)

ifndef ASAN
WSTACK = -Wstack-usage=80 -fstack-usage
endif

.PHONY: all
all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h
	$(CC) $(CFLAGS) $(WSTACK) -c -o mtojson.o mtojson.c

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(CFLAGS) -o test_mtojson test_mtojson.o mtojson.o

.PHONY: example
example: mtojson.o example.c
	$(CC) -Werror $(CFLAGS) -DOBJECT -o $@_OBJECT $^
	$(CC) -Werror $(CFLAGS) -DARRAY -o $@_ARRAY $^
	$(CC) -Werror $(CFLAGS) -DC_ARRAY -o $@_C_ARRAY $^
	$(CC) -Werror $(CFLAGS) -DPRIMITIVE -o $@_PRIMITIVE $^

.PHONY: clean
clean:
	rm -f mtojson.o test_mtojson.o test_mtojson mtojson.su example_*

.PHONY: cppcheck
cppcheck:
	cppcheck --suppress=missingIncludeSystem -I. --template gcc --enable=all --check-config *.[ch]

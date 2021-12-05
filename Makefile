CC ?= gcc

DEFAULTS = -std=c99
DEFAULTS += -g
DEFAULTS += -O2
DEFAULTS += -Wall

ifdef DEVELOPER
DEFAULTS += -Wextra
DEFAULTS += -Wpedantic

DEFAULTS += -Wconversion
DEFAULTS += -Wformat=2
DEFAULTS += -Wmissing-declarations
DEFAULTS += -Wnull-dereference
DEFAULTS += -Wshadow
DEFAULTS += -Wstrict-prototypes
DEFAULTS += -Wvla

ASAN = -fsanitize=address,undefined -fno-omit-frame-pointer
ifndef ASAN
WSTACK = -Wstack-usage=80 -fstack-usage
endif

GCCFLAGS += -Wduplicated-cond
GCCFLAGS += -Wjump-misses-init
endif

BUILD_FLAGS += $(ASAN)
BUILD_FLAGS += $(DEFAULTS)
BUILD_FLAGS += $(GCCFLAGS)
BUILD_FLAGS += $(CFLAGS)

.PHONY: all
all: mtojson.o test_mtojson
	@./test_mtojson

mtojson.o: mtojson.c mtojson.h
	$(CC) $(WSTACK) $(BUILD_FLAGS) -c -o mtojson.o mtojson.c

%.o: %.c
	$(CC) $(BUILD_FLAGS) -c -o $@ $<

test_mtojson: test_mtojson.o mtojson.o
	$(CC) $(BUILD_FLAGS) -o test_mtojson test_mtojson.o mtojson.o

.PHONY: example
example: mtojson.o example.c
	$(CC) -Werror $(BUILD_FLAGS) -DOBJECT -o $@_OBJECT $^
	$(CC) -Werror $(BUILD_FLAGS) -DARRAY -o $@_ARRAY $^
	$(CC) -Werror $(BUILD_FLAGS) -DC_ARRAY -o $@_C_ARRAY $^
	$(CC) -Werror $(BUILD_FLAGS) -DPRIMITIVE -o $@_PRIMITIVE $^

.PHONY: clean
clean:
	rm -f mtojson.o test_mtojson.o test_mtojson mtojson.su example_*

.PHONY: cppcheck
cppcheck:
	cppcheck --suppress=missingIncludeSystem -I. --template gcc --enable=all --check-config *.[ch]

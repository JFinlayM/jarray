# Builds and runs the tests with AddressSanitizer and UBSan (a leak or an invalid access makes the run fail).
CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer
SRCS = test_jarray.c ../src/jarray.c $(wildcard ../src/jarray_presets/*.c)

all: test

test_jarray: $(SRCS) ../inc/jarray.h
	$(CC) $(CFLAGS) $(SRCS) -o $@ -pthread

# allocator_may_return_null: the "huge size" tests ask for impossible allocations on purpose
test: test_jarray
	ASAN_OPTIONS=allocator_may_return_null=1 ./test_jarray

clean:
	rm -f test_jarray

.PHONY: all test clean

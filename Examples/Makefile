# Compiler and options
CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11
LDFLAGS = -ljarray

# All the .c files of the folder
SRCS = $(wildcard *.c)
# Each .c becomes an executable without extension
TARGETS = $(SRCS:.c=)

all: $(TARGETS)

# General rule: compile each .c into an executable
%: %.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: all clean

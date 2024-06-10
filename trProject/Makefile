# Makefile for compiling my_client and my_server

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2

# Targets
TARGETS = my_client my_server

# Source files
SRC_CLIENT = my_client.c
SRC_SERVER = my_server.c

# Object files
OBJ_CLIENT = $(SRC_CLIENT:.c=.o)
OBJ_SERVER = $(SRC_SERVER:.c=.o)

# Default target
all: $(TARGETS)

# Client target
my_client: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $@ $^

# Server target
my_server: $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $@ $^

# Clean target
clean:
	rm -f $(TARGETS) $(OBJ_CLIENT) $(OBJ_SERVER)

# Dependencies
my_client.o: my_client.c
my_server.o: my_server.c

.PHONY: all clean


CC = gcc
CFLAGS = -Wall -Wextra -pthread
LDFLAGS = -lrt

# Define the executables
EXECUTABLES = main

# Default target
all: $(EXECUTABLES)

# Linking rules
main: main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilation rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f *.o $(EXECUTABLES)

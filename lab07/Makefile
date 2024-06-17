# Makefile

CC = gcc
CFLAGS = -lpthread

# Targets
all: file_access create_records

# Compile file_access.c
file_access: file_access.c
	$(CC) -o file_access file_access.c

# Compile create_records.c
create_records: create_records.c
	$(CC) -o create_records create_records.c

# Clean up the build
clean:
	rm -f file_access create_records records.dat

.PHONY: all clean

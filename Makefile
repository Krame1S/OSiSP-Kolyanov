CC=gcc
CFLAGS = -std=c11 -pedantic -Wall -Wextra -Werror
CFILES=main.c
OBJECTS=main.o

BINARY=dirwalk

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

%.o:%.c
	$(CC) -c -o $@ $^

clean:
	rm -rf $(BINARY) $(OBJECTS)
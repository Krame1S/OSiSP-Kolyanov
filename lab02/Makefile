CC=gcc
CFLAGS=-W -Wall -Wno-unused-parameter -Wno-unused-variable -std=c11 -pedantic
CFILES=parent.c child.c
OBJECTS=$(CFILES:.c=.o)

BINARIES=parent child

all: $(BINARIES)

$(BINARIES): %: %.o
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BINARIES) $(OBJECTS)
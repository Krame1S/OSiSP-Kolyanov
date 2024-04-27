CC=gcc
CFILES=main.c funcs.c
OBJECTS=main.o funcs.o

BINARY=lab3

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(CC) -o $@ $^

%.o:%.c
	$(CC) -c -o $@ $^

clean:
	rm -rf $(BINARY) $(OBJECTS)
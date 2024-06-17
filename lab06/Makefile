CC = gcc

CFLAGS = -Wall -Wextra -O2 -pthread

TARGETS = generate_index sort_index check_sorted

SRC_GENERATE = generate_index.c
SRC_SORT = sort_index.c
SRC_CHECK = check_sorted.c

OBJ_GENERATE = $(SRC_GENERATE:.c=.o)
OBJ_SORT = $(SRC_SORT:.c=.o)
OBJ_CHECK = $(SRC_CHECK:.c=.o)

all: $(TARGETS)

generate_index: $(OBJ_GENERATE)
	$(CC) $(CFLAGS) -lm -o $@ $^

sort_index: $(OBJ_SORT)
	$(CC) $(CFLAGS) -o $@ $^

check_sorted: $(OBJ_CHECK)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) $(OBJ_GENERATE) $(OBJ_SORT) $(OBJ_CHECK)

.PHONY: all clean

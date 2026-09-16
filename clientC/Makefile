CC = clang
CFLAGS = -g -Wall -Wextra -Werror -gdwarf-4

BIN_DIR = bin
EXEC = $(BIN_DIR)/client-tuto-fort

all: $(EXEC)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(EXEC): client-tuto-fort.o client.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

client-tuto-fort.o: client-tuto-fort.c client.h
client.o: client.c client.h

clean:
	rm -f *.o

clear: clean
	rm -rf $(BIN_DIR)

.PHONY: all clean clear

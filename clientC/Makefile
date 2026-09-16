CC = clang
CFLAGS = -g -Wall -Wextra -Werror -gdwarf-4

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

EXEC = $(BIN_DIR)/client-tuto-fort

OBJS = $(OBJ_DIR)/client-tuto-fort.o \
       $(OBJ_DIR)/client.o

all: $(EXEC)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/client-tuto-fort.o: $(SRC_DIR)/client-tuto-fort.c $(SRC_DIR)/client.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/client.o: $(SRC_DIR)/client.c $(SRC_DIR)/client.h | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(EXEC)
	reset&&./$(EXEC)

clean:
	rm -rf $(OBJ_DIR) && reset

clear: clean
	rm -rf $(BIN_DIR) && reset

.PHONY: all run clean clear

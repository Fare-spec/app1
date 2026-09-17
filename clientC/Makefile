CC = clang
CFLAGS = -g -Wall -Wextra -Werror -gdwarf-4 -Isrc

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

# Tous les programmes client-*.c
PROGRAM_SRCS = $(filter-out \
	$(SRC_DIR)/client-crypteMove.c, \
	$(wildcard $(SRC_DIR)/client-*.c))

# src/client-truc.c -> bin/client-truc
PROGRAMS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(PROGRAM_SRCS))

# Objets communs à tous les programmes
COMMON_OBJS = $(OBJ_DIR)/client.o \
	      $(OBJ_DIR)/client-crypteMove.o

all: $(PROGRAMS)

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# Chaque client-XXX.c devient un exécutable bin/client-XXX
$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Compilation générique des .c vers build/*.o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

clear: clean
	rm -rf $(BIN_DIR)

.PHONY: all clean clear

CC = clang
CFLAGS = -g -Wall -Wextra -Werror -gdwarf-4 -Isrc

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

PROGRAM_SRCS = $(filter-out \
	$(SRC_DIR)/client-crypteMove.c, \
	$(wildcard $(SRC_DIR)/client-*.c))

PROGRAMS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(PROGRAM_SRCS))

COMMON_OBJS = $(OBJ_DIR)/client.o \
	      $(OBJ_DIR)/client-crypteMove.o

all: $(PROGRAMS)

# Build this timed challenge independently with optimization enabled.
onemillion: $(BIN_DIR)/client-OneMillion

$(BIN_DIR)/client-OneMillion: $(SRC_DIR)/client-OneMillion.c $(SRC_DIR)/onemillion-net.c $(SRC_DIR)/client.c $(SRC_DIR)/client-crypteMove.c $(SRC_DIR)/client.h $(SRC_DIR)/client-crypteMove.h | $(BIN_DIR)
	$(CC) $(CFLAGS) -O3 $(filter-out $(SRC_DIR)/client.c,$(filter %.c,$^)) -o $@

$(BIN_DIR)/client-NorthWoods: $(OBJ_DIR)/client-cryptseq-lib.o

$(BIN_DIR)/client-LostCause: $(OBJ_DIR)/client-cryptseq-lib.o

$(BIN_DIR)/client-LostCause2: $(OBJ_DIR)/client-cryptseq-lib.o

$(BIN_DIR)/client-planB: $(OBJ_DIR)/client-projetX-lib.o

$(OBJ_DIR)/client-projetX-lib.o: $(SRC_DIR)/client-projetX.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -DCLIENT_PROJETX_LIBRARY -c $< -o $@

$(OBJ_DIR)/client-cryptseq-lib.o: $(SRC_DIR)/client-cryptseq.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -DCLIENT_CRYPTSEQ_LIBRARY -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

$(BIN_DIR)/%: $(OBJ_DIR)/%.o $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

clear: clean
	rm -rf $(BIN_DIR)

.PHONY: all clean clear onemillion

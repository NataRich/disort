CC := gcc
CSTD := gnu99
INC_DIR := include
BIN_DIR := bin
SRC_DIR := src
OBJ_DIR := $(BIN_DIR)/obj
CFLAGS := -c -Wall -I$(INC_DIR)/  # -I shortens the include path to header files
COFLAG := -O2
CSTD := gnu99

SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/net.c $(SRC_DIR)/utils.c 
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)  # substitution

NAME := spawn
OUT := $(BIN_DIR)/$(NAME)

all: $(OUT)

# $@ replaces with the entire left part of :
# $^ replaces with the entire right part of :
$(OUT): $(OBJS)
	$(CC) -o $@ $(COFLAG) -std=$(CSTD) $^

# $< replaces with the first item in the dependency list
$(OBJ_DIR)/main.o: $(SRC_DIR)/main.c $(INC_DIR)/net.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

$(OBJ_DIR)/net.o: $(SRC_DIR)/net.c $(INC_DIR)/net.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

.PHONY: clean
clean:
	rm -f *.dat
	rm -f ./$(OUT)
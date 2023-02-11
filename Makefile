CC := gcc
CSTD := gnu99
INC_DIR := include
BIN_DIR := bin
SRC_DIR := src
OBJ_DIR := $(BIN_DIR)/obj
CFLAGS := -c -Wall -I$(INC_DIR)/  # -I shortens the include path to header files
COFLAG := -O2
CSTD := gnu99

SRCS := $(SRC_DIR)/main.c
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)  # substitution
DEPS := $(SRCS:$(SRC_DIR)/%.c=$(INC_DIR)/%.h)  # substitution

MASNAME := master
MASOUT := $(BIN_DIR)/$(MASNAME)

all: $(MASOUT)

# $@ replaces with the entire left part of :
# $^ replaces with the entire right part of :
$(MASOUT): $(OBJS)
	$(CC) -o $@ $(COFLAG) -std=$(CSTD) $^

# $< replaces with the first item in the dependency list
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

.PHONY: clean
clean:
	rm -f *.dat
	rm -f ./$(MASOUT)
CC := gcc
CSTD := gnu99
INC_DIR := include
BIN_DIR := bin
SRC_DIR := src
OBJ_DIR := $(BIN_DIR)/obj
CFLAGS := -c -Wall -I$(INC_DIR)/  # -I shortens the include path to header files
COFLAG := -O2
CSTD := gnu99

CSRCS := $(SRC_DIR)/cnode.c $(SRC_DIR)/net.c $(SRC_DIR)/utils.c $(SRC_DIR)/proto.c \
		 $(SRC_DIR)/files.c
DSRCS := $(SRC_DIR)/dnode.c $(SRC_DIR)/net.c $(SRC_DIR)/utils.c $(SRC_DIR)/proto.c \
		 $(SRC_DIR)/files.c
COBJS := $(CSRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)  # substitution
DOBJS := $(DSRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)  # substitution

CNAME := cnode
DNAME := dnode
COUT := $(BIN_DIR)/$(CNAME)
DOUT := $(BIN_DIR)/$(DNAME)

all: $(DOUT) $(COUT)

# $@ replaces with the entire left part of :
# $^ replaces with the entire right part of :
$(DOUT): $(DOBJS)
	$(CC) -o $@ $(COFLAG) -pthread -std=$(CSTD) $^

$(COUT): $(COBJS)
	$(CC) -o $@ $(COFLAG) -pthread -std=$(CSTD) $^

# $< replaces with the first item in the dependency list
$(OBJ_DIR)/dnode.o: $(SRC_DIR)/dnode.c $(INC_DIR)/net.h $(INC_DIR)/proto.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -pthread -std=$(CSTD) $< -o $@

$(OBJ_DIR)/cnode.o: $(SRC_DIR)/cnode.c $(INC_DIR)/net.h $(INC_DIR)/proto.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -pthread -std=$(CSTD) $< -o $@

$(OBJ_DIR)/proto.o: $(SRC_DIR)/proto.c $(INC_DIR)/proto.h $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

$(OBJ_DIR)/utils.o: $(SRC_DIR)/utils.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

$(OBJ_DIR)/files.o: $(SRC_DIR)/files.c $(INC_DIR)/utils.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

$(OBJ_DIR)/net.o: $(SRC_DIR)/net.c $(INC_DIR)/net.h
	$(CC) $(CFLAGS) -std=$(CSTD) $< -o $@

.PHONY: clean
clean:
	rm -f *.dat
	rm -f ./$(DOUT) ./$(COUT)
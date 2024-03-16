#CC=CC
#CFLAGS=-c -Ofast
#LDFLAGS=-lm -Ofast -w -fopenmp
#INC=./include
#SRC=$(INC)/read_dump_file.c
#OBJ=$(INC)/read_dump_file.o
#EXE=main
#
#all: $(SRC) $(EXE)
#
#$(EXE): $(OBJ)
#		$(CC) $(OBJ) -o $@ $(LDFLAGS)
#.c.o:
#		$(CC) $(LDFLAGS) $(CFLAGS) $< -o $@
#clean:
#		rm *.o $(INC)/*.o

CC := mpicc
SRCD := src
TSTD := tests
BLDD := build
BIND := bin
INCD := include
LIBD := lib

EXEC := main

MAIN := $(BLDD)/main.o

ALL_SRCF := $(wildcard $(SRCD)/*.c)
ALL_LIBF := $(wildcard $(LIBD)/*.o)
ALL_OBJF := $(patsubst $(SRCD)/%, $(BLDD)/%, $(ALL_SRCF:.c=.o))
ALL_FUNCF := $(filter-out $(MAIN), $(ALL_OBJF))

INC := -I $(INCD)

LIBS := $(LIBD)

CFLAGS := -g

.PHONY: clean all setup

all: setup $(BIND)/$(EXEC)

setup: $(BIND) $(BLDD)
$(BIND):
	mkdir -p $(BIND)

$(BLDD):
	mkdir -p $(BLDD)

$(BIND)/$(EXEC): $(MAIN) $(ALL_FUNCF)
	$(CC) $(MAIN) $(ALL_FUNCF) -o $(BIND)/$(EXEC)

$(BLDD)/%.o: $(SRCD)/%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

clean:
	rm -rf $(BLDD) $(BIND)
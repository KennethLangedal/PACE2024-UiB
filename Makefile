SHELL = /bin/bash

CC = gcc
CFLAGS = -g -std=gnu17 -O3 -march=native -I include 

OBJ_EXACT = main_exact.o graph.o ocm.o bnb.o heuristics.o dfas.o
OBJ_HEURISTIC = main_heuristic.o graph.o heuristics.o ocm.o
OBJ_UTIL = main_util.o graph.o ocm.o heuristics.o dfas.o tc.o

OBJ_EXACT := $(addprefix bin/, $(OBJ_EXACT))
OBJ_HEURISTIC := $(addprefix bin/, $(OBJ_HEURISTIC))
OBJ_UTIL := $(addprefix bin/, $(OBJ_UTIL))

DEP = $(OBJ_EXACT) $(OBJ_HEURISTIC) $(OBJ_UTIL)
DEP := $(sort $(DEP))

vpath %.c src
vpath %.h include

all : exact heuristic util

-include $(DEP:.o=.d)

exact : $(OBJ_EXACT)
	g++ -o $@ $^ -L../uwrmaxsat/build/release/lib -luwrmaxsat -L../cominisatps/build/release/lib -lcominisatps -lm -lz -lgmp --static

heuristic : $(OBJ_HEURISTIC)
	$(CC) $(CFLAGS) -o $@ $^ -lm

util : $(OBJ_UTIL)
	g++ -o $@ $^ bin/libipamirEvalMaxSAT2022.a -lm -lz

bin/%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -f exact heuristic util
	rm -f $(DEP)
	rm -f $(DEP:.o=.d)
SHELL = /bin/bash

CC = gcc
# CFLAGS = -g -std=gnu17 -O3 -march=haswell -I include -D _GNU_SOURCE -static
CFLAGS = -g -std=gnu17 -O3 -march=native -I include -D _GNU_SOURCE

OBJ_EXACT = main_exact.o ocm.o dfas.o tiny_solver.o heuristics.o exact.o cycle_packing.o
OBJ_HEURISTIC = main_heuristic.o ocm.o dfas.o tiny_solver.o heuristics.o

OBJ_EXACT := $(addprefix bin/, $(OBJ_EXACT))
OBJ_HEURISTIC := $(addprefix bin/, $(OBJ_HEURISTIC))

DEP = $(OBJ_EXACT) $(OBJ_HEURISTIC)
DEP := $(sort $(DEP))

vpath %.c src
vpath %.h include

all : exact heuristic

-include $(DEP:.o=.d)

exact : $(OBJ_EXACT)
#	g++ $(CFLAGS) -o $@ $^ bin/libipamirEvalMaxSAT2022.a -lm -lz -lgmp
	g++ $(CFLAGS) -o $@ $^ bin/libuwrmaxsat.a bin/libcominisatps.a bin/libmaxpre.a -lm -lz -lgmp

heuristic : $(OBJ_HEURISTIC)
	$(CC) $(CFLAGS) -o $@ $^ -lm

bin/%.o : %.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

.PHONY : clean
clean :
	rm -f exact heuristic
	rm -f $(DEP)
	rm -f $(DEP:.o=.d)

#g++ -o $@ $^ bin/libipamirEvalMaxSAT2022.a -lm -lz --static
#g++ -o $@ $^ -L../uwrmaxsat/build/release/lib -luwrmaxsat -L../cominisatps/build/release/lib -lcominisatps -lm -lz -lgmp --static

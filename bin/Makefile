SHELL = /bin/bash

CC = gcc
INCLUDE_DIRECTORIES = ../include
SRC_DIRECTORIES = ../src
CFLAGS = -g -std=gnu17 -O3 -march=native -MMD $(addprefix -I ,$(INCLUDE_DIRECTORIES))
LDFLAGS = -lm
OBJECTS = graph.o ocm.o

TARGET_EXACT = pace_exact
TARGET_HEURISTIC = pace_heuristic
TARGET_UTIL = pace_util

DEPS = $(OBJECTS:.o=.d) main_exact.d main_heuristic.d main_util.d

vpath %.c $(SRC_DIRECTORIES)
vpath %.h $(INCLUDE_DIRECTORIES)

all : $(TARGET_EXACT) $(TARGET_HEURISTIC) $(TARGET_UTIL)

$(TARGET_EXACT) : $(OBJECTS) main_exact.o
	$(CC) $(CFLAGS) -o ../$(TARGET_EXACT) $(OBJECTS) main_exact.o $(LDFLAGS)

$(TARGET_HEURISTIC) : $(OBJECTS) main_heuristic.o
	$(CC) $(CFLAGS) -o ../$(TARGET_HEURISTIC) $(OBJECTS) main_heuristic.o $(LDFLAGS)

$(TARGET_UTIL) : $(OBJECTS) main_util.o
	$(CC) $(CFLAGS) -o ../$(TARGET_UTIL) $(OBJECTS) main_util.o $(LDFLAGS)

-include $(DEPS)

.PHONY : clean
clean :
	rm -f ../$(TARGET_EXACT) ../$(TARGET_HEURISTIC) ../$(TARGET_UTIL)
	ls | grep -xv "Makefile" | xargs rm -f
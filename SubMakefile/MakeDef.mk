# directory def
SRC_DIR := $(LEVEL_DIR)src/
BUILD_DIR := $(LEVEL_DIR)build/
INCLUDE_DIR := $(LEVEL_DIR)include/
VPATH := $(INCLUDE_DIR)

# source, object def
OBJ_DIR := $(BUILD_DIR)$(shell pwd | sed 's|[[:print:]]*/$(subst $(LEVEL_DIR),,$(SRC_DIR))||g')/
SOURCES := $(wildcard *.c)
TARGET := $(addprefix $(OBJ_DIR), $(SOURCES:.c=.o))
DTARGET := $(SOURCES:.c=.d)

# compile def
CC := gcc
#CC := /usr/bin/gcc-6
OPTFLAG := -O2
#OPTFLAG := -g

OPTION := -D _GNU_SOURCE -std=gnu99 -Wall -Wno-unused-function $(OPTFLAG) $(OPENGL) -march=native
LIBS := -lm -pthread -lpthread

CFLAGS := -c $(OPTION) $(addprefix -I, $(INCLUDE_DIR)) 
LFLAGS := $(OPTION) $(LIBS)

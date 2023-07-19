BIN = v1
CC = gcc
CXX = g++
# CFLAGS = -O2 -Wall -Wextra -g -fPIC -I $(INCLUDE)
CFLAGS = -O2 -Wall -Wextra -fPIC -I $(INCLUDE)
CXXFLAGS = 
CPPFLAGS = 
LDFLAGS = 
INCLUDE = /Users/shinej/cs/research/prod-cons/uthash/src

# SRC_DIR = 
# SOURCES = v1.c
# OBJS = 
RM = rm -f
all: $(BIN)
.PHONY: all

# v1: ./v1.c
# 	$(CC) -o $@ $^ $(CFLAGS) 

v1: ./src/v1.c
	$(CC) -o $@ $^ $(CFLAGS) 


clean:
	$(RM) $(BIN)
	$(RM) $(OBJS)
.PHONY: clean
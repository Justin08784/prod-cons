BIN = v2
CC = gcc
CXX = g++
# CFLAGS = -O2 -Wall -Wextra -g -fPIC -I $(INCLUDE)
CFLAGS = -O2 -Wall -Wextra -fPIC -I $(INCLUDE)
CXXFLAGS = -std=c++20
CPPFLAGS = 
LDFLAGS = 
INCLUDE = ./uthash-src

# SRC_DIR = 
# SOURCES = v1.c
# OBJS = 
RM = rm -f
all: $(BIN)
.PHONY: all

ppv1: ./src/v1.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS) 

v1: ./src/v1.c
	$(CC) -o $@ $^ $(CFLAGS) 

v2: ./src/v2.c
	$(CC) -o $@ $^ $(CFLAGS) 


clean:
	$(RM) $(BIN)
	$(RM) $(OBJS)
.PHONY: clean
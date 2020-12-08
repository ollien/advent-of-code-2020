CC=g++
BIN_NAME=day8
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day8.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day8.cpp


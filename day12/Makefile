CC=g++
BIN_NAME=day12
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day12.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day12.cpp


CC=g++
BIN_NAME=day9
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day9.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day9.cpp


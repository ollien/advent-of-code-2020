CC=g++
BIN_NAME=day21
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day21.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day21.cpp


CC=g++
BIN_NAME=day18
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day18.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day18.cpp


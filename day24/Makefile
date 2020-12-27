CC=g++
BIN_NAME=day24
CCFLAGS=-o $(BIN_NAME) -g -O2 -std=c++17
LDFLAGS=

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day24.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day24.cpp


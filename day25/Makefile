CC=g++
BIN_NAME=day25
CCFLAGS=-o $(BIN_NAME) -g -O2 -std=c++17
LDFLAGS=

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day25.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day25.cpp


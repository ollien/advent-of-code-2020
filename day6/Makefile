CC=g++
BIN_NAME=day6
CCFLAGS=-o $(BIN_NAME) -g
LDFLAGS=-std=c++17

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day6.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day6.cpp


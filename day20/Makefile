CC=g++
BIN_NAME=day20
CCFLAGS=-o $(BIN_NAME) -O2 -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day20.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day20.cpp


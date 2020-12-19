CC=g++
BIN_NAME=day19
CCFLAGS=-o $(BIN_NAME) -g -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day19.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day19.cpp


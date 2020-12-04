CC=g++
BIN_NAME=day4
CCFLAGS=-o $(BIN_NAME)
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME): day4.cpp
	$(CC) $(CCFLAGS) $(LDFLAGS) day4.cpp


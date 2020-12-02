CC=g++
BIN_NAME=day2
CCFLAGS=-o $(BIN_NAME)
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(BIN_NAME)

clean:
	rm -f $(BIN_NAME)

$(BIN_NAME):
	$(CC) $(CCFLAGS) $(LDFLAGS) day2.cpp


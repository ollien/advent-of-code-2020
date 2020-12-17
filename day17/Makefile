CC=g++
PART_1_BIN_NAME=day17p1
PART_2_BIN_NAME=day17p2
CCFLAGS=-g -std=c++17
LDFLAGS=-lfolly

.PHONY: all, clean

all: $(PART_1_BIN_NAME) $(PART_2_BIN_NAME)

clean:
	rm -f $(PART_1_BIN_NAME) $(PART_2_BIN_NAME)

$(PART_1_BIN_NAME): day17p1.cpp
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) day17p1.cpp

$(PART_2_BIN_NAME): day17p2.cpp
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) day17p2.cpp


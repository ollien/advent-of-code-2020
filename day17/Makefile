CC=g++
PART_1_BIN_NAME=day17p1
PART_2_BIN_NAME=day17p2
CCFLAGS=-O2 -std=c++17
LDFLAGS=

.PHONY: all, clean

all: $(PART_1_BIN_NAME) $(PART_2_BIN_NAME)

clean:
	rm -f $(PART_1_BIN_NAME) $(PART_2_BIN_NAME)

$(PART_1_BIN_NAME): day17p1.cpp common.hpp
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) day17p1.cpp

$(PART_2_BIN_NAME): day17p2.cpp common.hpp
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) day17p2.cpp


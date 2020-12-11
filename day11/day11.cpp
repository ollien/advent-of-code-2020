#include <folly/String.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <queue>
#include <set>
#include <vector>

constexpr char EMPTY_CHAR = 'L';
constexpr char FLOOR_CHAR = '.';
constexpr char OCCUPIED_CHAR = '#';

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::vector<char> getNeighbors(const std::vector<std::string> &state, int row, int column) {
	std::vector<char> neighbors;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			if (dRow == 0 && dCol == 0) {
				continue;
			}

			int candidateRow = row + dRow;
			int candidateCol = column + dCol;
			if (candidateRow < 0 || candidateRow >= state.size() || candidateRow < 0 ||
				candidateCol >= state.at(candidateRow).size()) {
				continue;
			}

			char neighbor = state.at(candidateRow).at(candidateCol);
			neighbors.push_back(neighbor);
		}
	}

	return neighbors;
}

char applyRules(const std::vector<std::string> &state, int row, int column) {
	std::vector<char> neighbors = getNeighbors(state, row, column);
	char item = state.at(row).at(column);
	int numOccupied =
		std::count_if(neighbors.begin(), neighbors.end(), [](char neighbor) { return neighbor == OCCUPIED_CHAR; });
	if (item == EMPTY_CHAR && numOccupied == 0) {
		return OCCUPIED_CHAR;
	} else if (item == OCCUPIED_CHAR && numOccupied >= 4) {
		return EMPTY_CHAR;
	} else {
		return item;
	}
}

void printState(const std::vector<std::string> &state) {
	for (const std::string &row : state) {
		std::cout << row << std::endl;
	}

	std::cout << " " << std::endl;
}

int part1(const std::vector<std::string> &input) {
	std::vector<std::string> state(input);
	std::vector<std::string> nextState(input.size(), std::string(input.at(0).size(), ' '));
	while (state != nextState) {
		for (int i = 0; i < state.size(); i++) {
			for (int j = 0; j < input.at(i).size(); j++) {
				char res = applyRules(state, i, j);
				nextState.at(i).at(j) = res;
			}
		}

		std::swap(state, nextState);
	}

	return std::accumulate(state.cbegin(), state.cend(), 0, [](int total, const std::string &row) {
		return total + std::count_if(row.cbegin(), row.cend(), [](char item) { return item == OCCUPIED_CHAR; });
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << part1(input) << std::endl;
}

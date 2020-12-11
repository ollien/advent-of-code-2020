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
constexpr int PART_1_OCCUPIED_THRESHOLD = 4;
constexpr int PART_2_OCCUPIED_THRESHOLD = 5;

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

void printState(const std::vector<std::string> &state) {
	for (const std::string &row : state) {
		std::cout << row << std::endl;
	}

	std::cout << " " << std::endl;
}

std::vector<char> getPart1Neighbors(const std::vector<std::string> &state, int row, int column) {
	std::vector<char> neighbors;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			if (dRow == 0 && dCol == 0) {
				continue;
			}

			int candidateRow = row + dRow;
			int candidateCol = column + dCol;
			if (candidateRow < 0 || candidateRow >= state.size() || candidateCol < 0 ||
				candidateCol >= state.at(candidateRow).size()) {
				continue;
			}

			char neighbor = state.at(candidateRow).at(candidateCol);
			neighbors.push_back(neighbor);
		}
	}

	return neighbors;
}

template <typename T>
int sign(T val) {
	// Taken from https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
	return (T(0) < val) - (val < T(0));
}

/**
 * Extend a pair in all directions it extends - e.g. (1, 0) becomes (2,0)
 * @tparam T The type that the pair holds
 * @param vec The vector to extend
 */
template <typename T>
void extendVector(std::pair<T, T> &vec) {
	vec.first += sign(vec.first);
	vec.second += sign(vec.second);
}

std::vector<char> getPart2Neighbors(const std::vector<std::string> &state, int row, int column) {
	std::vector<char> neighbors;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			if (dRow == 0 && dCol == 0) {
				continue;
			}

			std::pair<int, int> deltaVector(dRow, dCol);
			char neighbor = FLOOR_CHAR;
			// Keep projecting our ray until we hit a non floor char
			while (neighbor == FLOOR_CHAR) {
				int candidateRow = row + deltaVector.first;
				int candidateCol = column + deltaVector.second;
				if (candidateRow < 0 || candidateRow >= state.size() || candidateCol < 0 ||
					candidateCol >= state.at(candidateRow).size()) {
					break;
				}

				neighbor = state.at(candidateRow).at(candidateCol);
				extendVector(deltaVector);
			}

			neighbors.push_back(neighbor);
		}
	}

	return neighbors;
}

char applyRules(const std::vector<char> &neighbors, char seatState, int occupiedThreshold) {
	int numOccupied =
		std::count_if(neighbors.begin(), neighbors.end(), [](char neighbor) { return neighbor == OCCUPIED_CHAR; });
	if (seatState == EMPTY_CHAR && numOccupied == 0) {
		return OCCUPIED_CHAR;
	} else if (seatState == OCCUPIED_CHAR && numOccupied >= occupiedThreshold) {
		return EMPTY_CHAR;
	} else {
		return seatState;
	}
}

int runSimulation(
	const std::vector<std::string> &input, int occupiedThreshold,
	std::function<std::vector<char>(const std::vector<std::string>, int, int)> getNeighbors) {
	std::vector<std::string> state(input);
	std::vector<std::string> nextState(input.size(), std::string(input.at(0).size(), ' '));
	for (int count = 0; state != nextState; count++) {
		for (int i = 0; i < state.size(); i++) {
			for (int j = 0; j < input.at(i).size(); j++) {
				std::vector<char> neighbors = getNeighbors(state, i, j);
				char res = applyRules(neighbors, state.at(i).at(j), occupiedThreshold);
				nextState.at(i).at(j) = res;
			}
		}

		printState(state);
		std::swap(state, nextState);
	}

	return std::accumulate(state.cbegin(), state.cend(), 0, [](int total, const std::string &row) {
		return total + std::count_if(row.cbegin(), row.cend(), [](char item) { return item == OCCUPIED_CHAR; });
	});
}

int part1(const std::vector<std::string> &input) {
	return runSimulation(input, PART_1_OCCUPIED_THRESHOLD, getPart1Neighbors);
}

int part2(const std::vector<std::string> &input) {
	return runSimulation(input, PART_2_OCCUPIED_THRESHOLD, getPart2Neighbors);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	// std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}

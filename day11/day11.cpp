#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>
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

/**
 * Get all of the neighbors for part 1
 * @param state The state of the board
 * @param row The row to get neighbors for
 * @param column The column to get neighbors for
 * @return std::vector<char> The neighbors of this position
 */
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
 * Extend a ray in all directions it extends - e.g. (1, 0) becomes (2,0)
 * @tparam T The type that the pair holds
 * @param ray The ray to extend
 */
template <typename T>
void extendRay(std::pair<T, T> &ray) {
	ray.first += sign(ray.first);
	ray.second += sign(ray.second);
}

/**
 * Cast the given ray until we hit a neighbor that is a seat
 * @param state The current board state
 * @param origin Where to start the ray from
 * @param ray The ray to extend until it hits a seat
 * @return optional<char> An optional of the neighbor that was hit. If none, this extended off the board.
 */
std::optional<char> castRayToSeat(
	const std::vector<std::string> &state, const std::pair<int, int> &origin, const std::pair<int, int> &ray) {
	std::pair<int, int> rayCursor(ray);
	char neighbor;
	// Keep projecting our ray until we hit a seat
	while (neighbor != OCCUPIED_CHAR && neighbor != EMPTY_CHAR) {
		int candidateRow = origin.first + rayCursor.first;
		int candidateCol = origin.second + rayCursor.second;
		if (candidateRow < 0 || candidateRow >= state.size() || candidateCol < 0 ||
			candidateCol >= state.at(candidateRow).size()) {
			return std::optional<char>();
		}

		neighbor = state.at(candidateRow).at(candidateCol);
		extendRay(rayCursor);
	}

	return std::optional<char>(neighbor);
}

/**
 * Get all of the neighbors for part 2
 * @param state The state of the board
 * @param row The row to get neighbors for
 * @param column The column to get neighbors for
 * @return std::vector<char> The neighbors of this position
 */
std::vector<char> getPart2Neighbors(const std::vector<std::string> &state, int row, int column) {
	std::pair<int, int> rayOrigin(row, column);
	std::vector<char> neighbors;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			if (dRow == 0 && dCol == 0) {
				continue;
			}

			std::pair<int, int> deltaRay(dRow, dCol);
			std::optional<char> neighbor = castRayToSeat(state, rayOrigin, deltaRay);
			if (neighbor.has_value()) {
				neighbors.push_back(*neighbor);
			}
		}
	}

	return neighbors;
}

/**
 * Apply the automata rules to a single seat
 * @param neighbors The neighbors of the location to check
 * @param seatState The state of the seat to check
 * @param occupiedThreshold How many seats must be occupied surrounding the location to the seat
 * @return char The new state for this position
 */
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

/**
 * Run the simulation to completion
 * @param input The input for the puzzle
 * @param occupiedThreshold The number of seats that need to be occupied surrounding a seat to empty it
 * @param getNeighbors A function to get the neighbors surrounding a position
 * @return int The puzzle answer
 */
int runSimulation(
	const std::vector<std::string> &input, int occupiedThreshold,
	std::function<std::vector<char>(const std::vector<std::string>, int, int)> getNeighbors) {
	std::vector<std::string> state(input);
	std::vector<std::string> nextState(input.size(), std::string(input.at(0).size(), ' '));
	while (state != nextState) {
		for (int i = 0; i < state.size(); i++) {
			for (int j = 0; j < input.at(i).size(); j++) {
				std::vector<char> neighbors = getNeighbors(state, i, j);
				char res = applyRules(neighbors, state.at(i).at(j), occupiedThreshold);
				nextState.at(i).at(j) = res;
			}
		}

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

	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}

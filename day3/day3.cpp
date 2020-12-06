#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

constexpr char TREE_CHAR = '#';

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
 * Find the number of trees that move along a path
 * @param input The input for the puzzle
 * @param xDelta The number of tiles to move right each step
 * @param yDelta The number of tiles to move down each step
 * @return int The number of trees encountered along this path
 */
int findNumTrees(const std::vector<std::string> &input, int xDelta, int yDelta = 1) {
	int xCursor = 0;
	int numTrees = 0;
	// I want to use an iterator here, but I don't have a way to cleanly make sure I don't blow through the end :(
	for (int i = 0; i < input.size(); i += yDelta) {
		const std::string &row = input.at(i);
		numTrees += (row.at(xCursor) == TREE_CHAR);
		xCursor = (xCursor + xDelta) % row.length();
	}

	return numTrees;
}

int part1(const std::vector<std::string> &input) {
	return findNumTrees(input, 3);
}

long part2(const std::vector<std::string> &input) {
	// deltas in the x and y directions respectively
	std::vector<std::pair<int, int>> deltas{
		std::pair<int, int>(1, 1),
		std::pair<int, int>(3, 1),
		std::pair<int, int>(5, 1),
		std::pair<int, int>(7, 1),
		std::pair<int, int>(1, 2),
	};

	return std::reduce(
		std::execution::par, deltas.begin(), deltas.end(), 1L, [&](long total, std::pair<int, int> step_deltas) {
			return total * findNumTrees(input, step_deltas.first, step_deltas.second);
		});
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

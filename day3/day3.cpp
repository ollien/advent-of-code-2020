#include <fstream>
#include <iostream>
#include <vector>

constexpr char TREE_CHAR = '#';

std::vector<std::string> read_input(const std::string &filename) {
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
	return 1L * findNumTrees(input, 1, 1) * findNumTrees(input, 3, 1) * findNumTrees(input, 5, 1) *
		   findNumTrees(input, 7, 1) * findNumTrees(input, 1, 2);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << "./" << argv[0] << "<input_file>" << std::endl;
		return 1;
	}

	auto input = read_input(argv[1]);
	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}

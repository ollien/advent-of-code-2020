#include <folly/String.h>

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <vector>

constexpr char IGNORE_CHAR = 'X';
constexpr auto MASK_PATTERN = R"(mask = ([X0-9]+))";
constexpr auto MEM_PATTERN = R"(mem\[(\d+)\] = (\d+))";

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::vector<int> parseStartingNumbers(const std::vector<std::string> &input) {
	std::vector<std::string> rawNumbers;
	folly::split(",", input.at(0), rawNumbers);
	std::vector<int> startingNumbers;

	std::transform(
		rawNumbers.cbegin(), rawNumbers.cend(), std::back_inserter(startingNumbers), [](const std::string &rawNumber) {
			return std::stoi(rawNumber);
		});

	return startingNumbers;
}

int solve(const std::vector<int> &startingNumbers, int max_n) {
	std::unordered_map<int, int> turnSpoken;
	int lastNumber = startingNumbers.back();
	// Skip past the starting numbers< put them at the proper place
	for (std::vector<int>::size_type turn = 1; turn < startingNumbers.size(); turn++) {
		turnSpoken.emplace(startingNumbers.at(turn - 1), turn);
	}

	// Keep passing on until the right n
	for (int turn = startingNumbers.size() + 1; turn <= max_n; turn++) {
		int number = 0;
		if (turnSpoken.find(lastNumber) != turnSpoken.end()) {
			number = (turn - 1) - turnSpoken.at(lastNumber);
		}

		turnSpoken[lastNumber] = turn - 1;
		lastNumber = number;
	}

	return lastNumber;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto startingNumbers = parseStartingNumbers(input);

	std::cout << solve(startingNumbers, 2020) << std::endl;
	std::cout << solve(startingNumbers, 30000000) << std::endl;
}

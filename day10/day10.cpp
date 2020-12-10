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

constexpr auto MAX_VOLTAGE_DELTA = 3;

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
 * Convert a vector of strings to a vector of numbers
 * @param input The input for the puzzle
 * @return std::vector<int> The puzzle input as numbers
 */
std::vector<int> convertInputToNumbers(const std::vector<std::string> &input) {
	std::vector<int> converted;
	converted.reserve(input.size());
	std::transform(input.cbegin(), input.cend(), std::back_inserter(converted), [](const std::string &line) {
		return std::stol(line);
	});

	return converted;
}

/**
 * Add the outlet and max voltage to the input list, and sort it
 * @param input The input for the puzzle
 */
void prepareInput(std::vector<int> &input) {
	int max_voltage = *std::max_element(input.cbegin(), input.cend()) + MAX_VOLTAGE_DELTA;
	input.push_back(0);
	input.push_back(max_voltage);
	std::sort(input.begin(), input.end());
}

template <typename Key, typename Value>
Value getOrDefault(const std::map<Key, Value> &map, Key key, Value defaultValue) {
	try {
		return map.at(key);
	} catch (std::out_of_range) {
		return defaultValue;
	}
}

int part1(const std::vector<int> &input) {
	std::map<int, int> differenceCounts;
	std::vector<int> adapters(input);
	prepareInput(adapters);

	for (auto it = adapters.cbegin(); it != adapters.end() && std::next(it) != adapters.cend(); it++) {
		auto difference = *std::next(it) - *it;
		differenceCounts[difference]++;
	}

	return getOrDefault(differenceCounts, 1, 0) * getOrDefault(differenceCounts, 3, 0);
}

long part2(const std::vector<int> &input) {
	std::vector<int> adapters(input);
	prepareInput(adapters);
	// Dynamic programming approach that counts the number of ways to get to each point by backshifting
	std::vector<long> counts(adapters.back() + 1, 0);
	counts.at(0) = 1;
	for (auto it = std::next(adapters.cbegin()); it != adapters.cend(); it++) {
		for (int i = *it; i >= 0 && i >= *it - MAX_VOLTAGE_DELTA; i--) {
			counts.at(*it) += counts.at(i);
		}
	}

	return counts.back();
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto numericInput = convertInputToNumbers(input);

	std::cout << part1(numericInput) << std::endl;
	std::cout << part2(numericInput) << std::endl;
}

#include <folly/String.h>

#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string_view>
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
	int max_voltage = *std::max_element(input.cbegin(), input.cend()) + MAX_VOLTAGE_DELTA;
	adapters.insert(adapters.begin(), 0);
	adapters.insert(adapters.begin(), max_voltage);
	std::sort(adapters.begin(), adapters.end());

	for (auto it = adapters.cbegin() + 1; it != adapters.cend(); it++) {
		auto difference = *it - *(it - 1);
		differenceCounts[difference]++;
	}

	return getOrDefault(differenceCounts, 1, 0) * getOrDefault(differenceCounts, 3, 0);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto numericInput = convertInputToNumbers(input);

	std::cout << part1(numericInput) << std::endl;
	// std::cout << part2(numbers, part1Answer) << std::endl;
}

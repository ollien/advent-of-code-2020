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

/**
 * Finds the number of paths from the given source node to the target (the end of the adapters lsit)
 * Based on algorithm from:
 * https://cs.stackexchange.com/questions/3078/algorithm-that-finds-the-number-of-simple-paths-from-s-to-t-in-g
 * @param adapters The puzzle input, prepared by prepareAdapters
 * @param numPaths A count of the number of paths
 * @param source The node to start from
 * @return long
 */
long solvePart2WithGraph(const std::vector<int> &adapters, std::unordered_map<int, long> &numPaths, int source = 0) {
	int target = adapters.size() - 1;
	if (source == target) {
		return 1;
	}

	// If we already know the number of paths to source, we're done
	auto pathsToSource = numPaths.find(source);
	if (pathsToSource != numPaths.end()) {
		return pathsToSource->second;
	}

	// Find the number of paths by just checking all off the neighbors
	// Something is defined as a neighbor if we can get to it by addding [1, 3]
	long total = 0;
	for (int i = source + 1; i < adapters.size() && adapters.at(i) - adapters.at(source) <= MAX_VOLTAGE_DELTA; i++) {
		total += solvePart2WithGraph(adapters, numPaths, i);
	}

	numPaths.emplace(source, total);

	return total;
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
	std::unordered_map<int, long> counts;

	return solvePart2WithGraph(adapters, counts);
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

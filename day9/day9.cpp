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

constexpr int PREAMBLE_SIZE = 25;

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
 * @return std::vector<long> The puzzle input as numbers
 */
std::vector<long> convertInputToNumbers(const std::vector<std::string> &input) {
	std::vector<long> converted;
	converted.reserve(input.size());
	std::transform(input.cbegin(), input.cend(), std::back_inserter(converted), [](const std::string &line) {
		return std::stol(line);
	});

	return converted;
}

long part1(const std::vector<long> &numbers) {
	for (auto totalIterator = numbers.cbegin() + PREAMBLE_SIZE; totalIterator != numbers.cend(); totalIterator++) {
		// Though we skip the first PREAMBLE_SIZE to find the number to sum to, we want to start summing as if we didn't
		// do that
		auto rangeStart = totalIterator - PREAMBLE_SIZE;
		auto rangeEnd = totalIterator;
		auto result = std::find_if(rangeStart, rangeEnd, [=](long candidate1) {
			auto candidate2 = std::find_if(
				rangeStart, rangeEnd, [=](long candidate2) { return candidate1 + candidate2 == *totalIterator; });

			return candidate2 != rangeEnd;
		});

		if (result == rangeEnd) {
			return *totalIterator;
		}
	}

	throw std::invalid_argument("No solution in input");
}

long part2(const std::vector<long> &numbers, long desired) {
	for (auto rangeStartIterator = numbers.begin(); rangeStartIterator != numbers.end(); rangeStartIterator++) {
		for (auto rangeEndIterator = rangeStartIterator + 1; rangeEndIterator != numbers.end(); rangeEndIterator++) {
			// Sum all the numbers between our two iterators
			auto total = std::accumulate(
				rangeStartIterator, rangeEndIterator, 0, [](int num, int total) { return num + total; });
			// If this total is our desired, we're done
			if (total == desired) {
				auto minIterator = std::min_element(rangeStartIterator, rangeEndIterator);
				auto maxIterator = std::max_element(rangeStartIterator, rangeEndIterator);
				return *minIterator + *maxIterator;
			}
		}
	}

	throw std::invalid_argument("No solution in input");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto numbers = convertInputToNumbers(input);

	auto part1Answer = part1(numbers);
	std::cout << part1Answer << std::endl;
	std::cout << part2(numbers, part1Answer) << std::endl;
}

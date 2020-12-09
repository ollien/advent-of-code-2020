#include <folly/String.h>

#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
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

std::vector<long> convertInputToNumbers(const std::vector<std::string> &input) {
	std::vector<long> converted;
	converted.reserve(input.size());
	std::transform(input.cbegin(), input.cend(), std::back_inserter(converted), [](const std::string &line) {
		return std::stol(line);
	});

	return converted;
}

/**
 * Find a number in the range that, it, plus candidate, equals total.
 * @tparam Iter
 * @param total The number to sum to
 * @param candidate The number to sum with
 * @param start The start of the range
 * @param end The end of the range
 * @return bool If such a number exists, return true, false otherwise.
 */
template <typename Iter>
bool numberThatSumsInRange(int total, int candidate, Iter start, Iter end) {
	for (Iter it = start; it != end; it++) {
		// std::cout << *it << std::endl;
		if (candidate + *it == total) {
			return true;
		}
	}

	return false;
}

long part1(const std::vector<long> &numbers) {
	for (auto checkIterator = numbers.begin() + PREAMBLE_SIZE; checkIterator != numbers.end(); checkIterator++) {
		auto rangeStart = checkIterator - PREAMBLE_SIZE;
		auto rangeEnd = checkIterator;
		std::optional<int> foundNumber;
		for (auto it = rangeStart; it != rangeEnd; it++) {
			if (numberThatSumsInRange(*checkIterator, *it, rangeStart, rangeEnd)) {
				foundNumber = *checkIterator;
				break;
			}
		}

		if (!foundNumber.has_value()) {
			return *checkIterator;
		}
	}

	throw std::invalid_argument("No solution in input");
}

long part2(const std::vector<long> &numbers) {
	long desired = part1(numbers);
	for (auto rangeStartIterator = numbers.begin(); rangeStartIterator != numbers.end(); rangeStartIterator++) {
		for (auto rangeEndIterator = rangeStartIterator + 1; rangeEndIterator != numbers.end(); rangeEndIterator++) {
			auto total = std::accumulate(
				rangeStartIterator, rangeEndIterator, 0, [](int num, int total) { return num + total; });
			if (total == desired) {
				auto min = std::min_element(rangeStartIterator, rangeEndIterator);
				auto max = std::max_element(rangeStartIterator, rangeEndIterator);
				return *min + *max;
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

	std::cout << part1(numbers) << std::endl;
	std::cout << part2(numbers) << std::endl;
}

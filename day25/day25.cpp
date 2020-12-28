#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <tuple>

constexpr int DIVIDEND = 20201227;
constexpr int START_SUBJECT_NUMBER = 7;

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
 * @param int The puzzle input
 * @return std::pair<long, long> Each public key
 */
std::pair<long, long> parseInput(const std::vector<std::string> &input) {
	assert(input.size() == 2);

	return std::make_pair(std::stol(input.at(0)), std::stol(input.at(1)));
}

/**
 * Perform the transform
 * @param subjectNumber The subject number
 * @param numLoops The number of loops
 * @param initValue The initial value to use
 * @return long The transform
 */
long performTransform(int subjectNumber, int numLoops, int initValue = 1) {
	long res = initValue;
	for (int i = 0; i < numLoops; i++) {
		res *= subjectNumber;
		res %= DIVIDEND;
	}

	return res;
}

/**
 * Brute force the loop size
 * @param subjectNumber The subject number
 * @param target The target number to stop at
 * @return int THe loop size
 */
int findLoopSize(int subjectNumber, int target) {
	long value = 1;
	for (int loopSize = 1;; loopSize++) {
		value = performTransform(subjectNumber, 1, value);
		if (value == target) {
			return loopSize;
		}
	}
}

long part1(const std::pair<int, int> &publicKeys) {
	int cardLoopSize = findLoopSize(START_SUBJECT_NUMBER, publicKeys.first);

	return performTransform(publicKeys.second, cardLoopSize);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = parseInput(input);

	std::cout << part1(parsedInput) << std::endl;
}

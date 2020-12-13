#include <folly/String.h>

#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

constexpr auto OUT_OF_SERVICE_BUS = "x";

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
 * Parse a split list of bus times into a vector of pairs of the form <busLeaveTime, offset>
 * @param rawBusTimes The bus times to parse
 * @return std::vector<std::pair<int, int>>
 */
std::vector<std::pair<int, int>> parseBusTimes(const std::vector<std::string> &rawBusTimes) {
	int nextBusOffsset = 0;
	// Holds both the bus time and its offset from the time t.
	std::vector<std::pair<int, int>> busTimes;
	busTimes.reserve(rawBusTimes.size());
	for (auto it = rawBusTimes.cbegin(); it != rawBusTimes.cend(); (it++, nextBusOffsset++)) {
		auto rawBusTime = *it;
		if (rawBusTime == OUT_OF_SERVICE_BUS) {
			continue;
		}

		int busTime = std::stoi(rawBusTime);
		busTimes.emplace_back(busTime, nextBusOffsset);
	}

	return busTimes;
}

/**
 * Parse the puzzle input
 * @param input The input to parse
 * @return std::pair<int, std::vector<std::pair<int, int>>> A pair of the form <busLeaveTime, <busLeaveTime, offset>>
 */
std::pair<int, std::vector<std::pair<int, int>>> parseInput(const std::vector<std::string> &input) {
	int startTime = std::stoi(input.at(0));
	const std::string &rawBusses = input.at(1);
	std::vector<std::string> rawBusTimes;
	folly::split(",", rawBusses, rawBusTimes);

	auto parsedBusTimes = parseBusTimes(rawBusTimes);

	return std::pair<int, std::vector<std::pair<int, int>>>(startTime, parsedBusTimes);
}

int part1(int busStartTime, const std::vector<std::pair<int, int>> &busTimes) {
	// Calculate the bus times that are immediately after the start time input
	std::map<int, int> nextBusses;
	std::transform(
		busTimes.cbegin(),
		busTimes.cend(),
		std::inserter(nextBusses, nextBusses.end()),
		[busStartTime](const std::pair<int, int> &busTime) {
			// Take the ceiling when dividing
			int numIntervals = (busTime.first + busStartTime - 1) / busTime.first;
			int nextBusTime = busTime.first * numIntervals;

			return std::pair<int, int>(busTime.first, nextBusTime);
		});

	auto minPairIterator = std::min_element(
		nextBusses.cbegin(), nextBusses.cend(), [](std::pair<int, int> time1, std::pair<int, int> time2) {
			return time1.second < time2.second;
		});

	int bestBusID = minPairIterator->first;
	int bestBusTime = minPairIterator->second;
	int timeToWait = bestBusTime - busStartTime;

	return bestBusID * timeToWait;
}

long part2(const std::vector<std::pair<int, int>> &busTimes) {
	auto cursor = busTimes.cbegin();
	long t = cursor->first;
	long stepSize = cursor->first;
	cursor++;
	while (cursor != busTimes.cend()) {
		t += stepSize;
		// Find the time that next bus that will start at the offset after t
		if ((t + cursor->second) % cursor->first != 0) {
			continue;
		}

		stepSize = std::lcm(stepSize, cursor->first);
		cursor++;
	}

	return t;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = parseInput(input);

	std::cout << part1(parsedInput.first, parsedInput.second) << std::endl;
	std::cout << part2(parsedInput.second) << std::endl;
}

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

std::pair<int, std::vector<std::string>> splitInput(const std::vector<std::string> &input) {
	int startTime = std::stoi(input.at(0));
	const std::string &rawBusses = input.at(1);
	std::vector<std::string> rawBusTimes;
	folly::split(",", rawBusses, rawBusTimes);

	return std::pair<int, std::vector<std::string>>(startTime, rawBusTimes);
}

int part1(int busStartTime, const std::vector<std::string> &rawBusTimes) {
	std::vector<int> busTimes;
	busTimes.reserve(rawBusTimes.size());
	for (const std::string &rawBusTime : rawBusTimes) {
		if (rawBusTime == OUT_OF_SERVICE_BUS) {
			continue;
		}

		int busTime = std::stoi(rawBusTime);
		busTimes.push_back(busTime);
	}

	// Calculate the bus times that are immediately after the start time input
	std::map<int, int> nextBusses;
	std::transform(
		busTimes.cbegin(), busTimes.cend(), std::inserter(nextBusses, nextBusses.end()), [busStartTime](int busTime) {
			// Take the ceiling when dividing
			int numIntervals = (busTime + busStartTime - 1) / busTime;
			int nextBusTime = busTime * numIntervals;

			return std::pair<int, int>(busTime, nextBusTime);
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

long part2(const std::vector<std::string> rawBusTimes) {
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
	auto parsedInput = splitInput(input);

	std::cout << part1(parsedInput.first, parsedInput.second) << std::endl;
	std::cout << part2(parsedInput.second) << std::endl;
}

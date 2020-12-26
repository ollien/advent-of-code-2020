#include <boost/hana/any_of.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/hana/for_each.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
#include <string>
#include <tuple>

constexpr int NUM_CRAB_TURNS = 100;

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::vector<int> makeCupList(const std::string &inputLine) {
	std::vector<int> cups;
	std::transform(inputLine.cbegin(), inputLine.cend(), std::back_inserter(cups), [](char rawNum) {
		if (rawNum < '0' || rawNum > '9') {
			throw std::invalid_argument("Not numeric");
		}

		return rawNum - '0';
	});

	return cups;
}

std::tuple<int, int, int> getPickedUpCups(int currentCupIndex, const std::vector<int> &cupList) {
	return std::make_tuple(
		cupList.at((currentCupIndex + 1) % cupList.size()),
		cupList.at((currentCupIndex + 2) % cupList.size()),
		cupList.at((currentCupIndex + 3) % cupList.size()));
}

template <typename Tuple, typename ValueType>
bool tupleContainsItem(Tuple tuple, ValueType value) {
	return boost::hana::any_of(tuple, [value](ValueType item) { return item == value; });
}

int findDestinationCup(int currentCupIndex, const std::vector<int> &cupList) {
	int currentCup = cupList.at(currentCupIndex);
	std::tuple<int, int, int> pickedUpCups = getPickedUpCups(currentCupIndex, cupList);
	auto cupMinMaxIterators = std::minmax_element(cupList.cbegin(), cupList.cend());
	std::pair<int, int> cupMinMax = std::make_pair(*(cupMinMaxIterators.first), *(cupMinMaxIterators.second));

	int destinationCup = currentCup;
	do {
		destinationCup--;
		if (destinationCup < cupMinMax.first) {
			destinationCup = cupMinMax.second;
		}
	} while (tupleContainsItem(pickedUpCups, destinationCup));

	return destinationCup;
}

std::string makeFullCupLabel(const std::vector<int> cups) {
	std::string output;
	output.reserve(cups.size() - 1);
	auto startIt = std::find(cups.cbegin(), cups.cend(), 1);
	for (int i = (startIt - cups.begin() + 1) % cups.size(); cups.at(i) != 1; i = (i + 1) % cups.size()) {
		output.push_back(cups.at(i) + '0');
	}

	return output;
}

std::string part1(const std::string &inputLine) {
	std::vector<int> cups = makeCupList(inputLine);
	int currentCup = cups.front();
	for (int i = 0; i < NUM_CRAB_TURNS; i++) {
		auto currentCupIt = std::find(cups.cbegin(), cups.cend(), currentCup);
		int destinationCup = findDestinationCup(currentCupIt - cups.cbegin(), cups);
		std::tuple<int, int, int> pickedUpCups = getPickedUpCups(currentCupIt - cups.cbegin(), cups);

		// Perform the cup moves
		std::vector<int> newCups;
		newCups.reserve(cups.size());
		for (int cup : cups) {
			if (tupleContainsItem(pickedUpCups, cup)) {
				continue;
			}

			newCups.push_back(cup);
			// Copy the existing cups over
			if (cup == destinationCup) {
				boost::hana::for_each(pickedUpCups, [&newCups](int item) { newCups.push_back(item); });
			}
		}

		// Get the new current cup
		cups = std::move(newCups);
		currentCupIt = std::find(cups.cbegin(), cups.cend(), currentCup);
		if (currentCupIt + 1 == cups.cend()) {
			currentCup = cups.front();
		} else {
			currentCup = *(currentCupIt + 1);
		}
	}

	return makeFullCupLabel(cups);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << part1(input.at(0)) << std::endl;
}

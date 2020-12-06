#include <folly/String.h>

#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <string_view>
#include <vector>

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
 * Separate the inputs into groups - each inner vector is a list of lines within that group (separated by newlines)
 * @param input The puzzle input
 * @return std::vector<std::vector<std::string>> The inputs grouped by newlines
 */
std::vector<std::vector<std::string>> getGroups(const std::vector<std::string> &input) {
	std::vector<std::vector<std::string>> groups;
	std::vector<std::string> currentGroup;
	for (std::string line : input) {
		if (line.length() == 0) {
			groups.push_back(currentGroup);
			currentGroup.clear();
			continue;
		}

		currentGroup.push_back(line);
	}

	groups.push_back(currentGroup);
	return groups;
}

int part1(const std::vector<std::vector<std::string>> &groups) {
	int total = 0;
	for (const std::vector<std::string> &group : groups) {
		std::set<char> groupAnswers;
		for (const std::string &personAnswers : group) {
			std::for_each(personAnswers.cbegin(), personAnswers.cend(), [&groupAnswers](char answer) {
				groupAnswers.insert(answer);
			});
		}
		total += groupAnswers.size();
	}

	return total;
}

int part2(const std::vector<std::vector<std::string>> &groups) {
	int total = 0;
	// Make a set of all possible answers, from a to z.
	std::set<char> allPossibleAnswers;
	for (char i = 'a'; i <= 'z'; i++) {
		allPossibleAnswers.insert(i);
	}

	for (const std::vector<std::string> &group : groups) {
		// Not technically needed for the problem but it would be incorrect if this happened :) total would add 26.
		if (group.size() == 0) {
			continue;
		}
		// Copy all possible answers so we can produce a set intersection with them
		std::set<char> commonAnswers(allPossibleAnswers);
		for (const std::string &rawPersonAnswers : group) {
			std::set<char> personAnswers;
			std::for_each(rawPersonAnswers.cbegin(), rawPersonAnswers.cend(), [&personAnswers](char answer) {
				personAnswers.insert(answer);
			});

			std::set<char> intersection;
			std::set_intersection(
				personAnswers.begin(),
				personAnswers.end(),
				commonAnswers.begin(),
				commonAnswers.end(),
				std::inserter(intersection, intersection.begin()));

			commonAnswers = std::move(intersection);
		}

		total += commonAnswers.size();
	}

	return total;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto groups = getGroups(input);
	std::cout << part1(groups) << std::endl;
	std::cout << part2(groups) << std::endl;
}

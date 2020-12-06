#include <folly/String.h>

#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <set>
#include <string_view>
#include <vector>

std::vector<std::string> read_input(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

int part1(const std::vector<std::string> &input) {
	int total = 0;
	std::set<char> groups;
	for (std::string_view line : input) {
		if (line.length() == 0) {
			total += groups.size();
			groups.clear();
			continue;
		}

		for (char item : line) {
			groups.insert(item);
		}
	}

	return total;
}

int part2(const std::vector<std::string> &input) {
	int total = 0;
	std::set<char> commonAnswers;
	for (char i = 'a'; i <= 'z'; i++) {
		commonAnswers.insert(i);
	}

	for (std::string_view line : input) {
		if (line.length() == 0) {
			total += commonAnswers.size();
			commonAnswers.clear();
			for (char i = 'a'; i <= 'z'; i++) {
				commonAnswers.insert(i);
			}
			continue;
		}

		std::set<char> personAnswers;
		for (char item : line) {
			personAnswers.insert(item);
		}

		std::set<char> intersection;
		std::set_intersection(
			personAnswers.begin(),
			personAnswers.end(),
			commonAnswers.begin(),
			commonAnswers.end(),
			std::inserter(intersection, intersection.begin()));

		std::string out;
		folly::join(" ", commonAnswers.begin(), commonAnswers.end(), out);
		std::cout << out << std::endl;
		commonAnswers = std::move(intersection);
	}

	return total;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = read_input(argv[1]);
	input.push_back("");
	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}

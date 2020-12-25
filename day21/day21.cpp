#include <folly/String.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <regex>
#include <set>
#include <string>
#include <vector>

using IngredientLineItem = std::pair<std::vector<std::string>, std::vector<std::string>>;
auto constexpr INGREDIENT_PATTERN = R"((.*) \(contains (.*)\))";

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::pair<std::string, std::string> splitInputLine(const std::string &inputLine) {
	std::regex pattern(INGREDIENT_PATTERN);
	std::smatch matches;
	if (!std::regex_match(inputLine, matches, pattern)) {
		throw std::invalid_argument("Invalid input line");
	}

	return std::make_pair(matches[1].str(), matches[2].str());
}

std::vector<IngredientLineItem> parseInput(const std::vector<std::string> &input) {
	std::vector<IngredientLineItem> res;
	std::transform(input.cbegin(), input.cend(), std::back_inserter(res), [](const std::string &inputLine) {
		auto lineComponents = splitInputLine(inputLine);
		std::vector<std::string> unknownIngredients;
		std::vector<std::string> allergens;
		folly::split(" ", lineComponents.first, unknownIngredients);
		folly::split(", ", lineComponents.second, allergens);

		return std::make_pair(unknownIngredients, allergens);
	});

	return res;
}

std::map<std::string, std::set<std::string>> correlateIngredients(const std::vector<IngredientLineItem> &input) {
	std::map<std::string, std::set<std::string>> knownCorrelations;
	std::vector<std::string> allIngredients;
	for (const IngredientLineItem &inputLine : input) {
		for (const std::string &allergen : inputLine.second) {
			std::set<std::string> &allergenCorrelations = knownCorrelations[allergen];
			std::set<std::string> allergenMatches(inputLine.first.cbegin(), inputLine.first.cend());
			if (allergenCorrelations.empty()) {
				allergenCorrelations = std::move(allergenMatches);
				continue;
			}

			std::set<std::string> intersection;
			std::set_intersection(
				allergenCorrelations.cbegin(),
				allergenCorrelations.cend(),
				allergenMatches.cbegin(),
				allergenMatches.cend(),
				std::inserter(intersection, intersection.end()));

			allergenCorrelations = std::move(intersection);
		}
	}

	return knownCorrelations;
}

int part1(const std::vector<IngredientLineItem> &input) {
	auto knownCorrelations = correlateIngredients(input);
	std::set<std::string> foundIngredients;
	for (const auto &correlationEntry : knownCorrelations) {
		foundIngredients.insert(correlationEntry.second.cbegin(), correlationEntry.second.cend());
	}

	std::vector<std::string> allIngredients;
	for (const IngredientLineItem &item : input) {
		allIngredients.insert(allIngredients.end(), item.first.cbegin(), item.first.cend());
	}

	std::set<std::string> allIngredientsSet(allIngredients.cbegin(), allIngredients.cend());
	std::set<std::string> unmatchedIngredients;
	std::set_difference(
		allIngredientsSet.cbegin(),
		allIngredientsSet.cend(),
		foundIngredients.cbegin(),
		foundIngredients.cend(),
		std::inserter(unmatchedIngredients, unmatchedIngredients.end()));

	return std::count_if(
		allIngredients.begin(), allIngredients.end(), [&unmatchedIngredients](const std::string &ingredient) {
			return unmatchedIngredients.find(ingredient) != unmatchedIngredients.end();
		});
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

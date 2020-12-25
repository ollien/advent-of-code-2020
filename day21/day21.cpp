#include <folly/String.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <regex>
#include <set>
#include <string>
#include <vector>

// Pair of ingredients and allergens
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

/**
 * Split an input line into its components of both the foreign ingredient and the allergens
 * @param inputLine The input line
 * @return std::pair<std::string, std::string> The ingredients and allergens as a pair
 */
std::pair<std::string, std::string> splitInputLine(const std::string &inputLine) {
	std::regex pattern(INGREDIENT_PATTERN);
	std::smatch matches;
	if (!std::regex_match(inputLine, matches, pattern)) {
		throw std::invalid_argument("Invalid input line");
	}

	return std::make_pair(matches[1].str(), matches[2].str());
}

/**
 * Parse the puzzle input
 * @param input The puzzle input
 * @return std::vector<IngredientLineItem> The line items for the ingredients
 */
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

/**
 * Correlate ingredients to their all
 * @param input The parsed puzzle input
 * @return std::map<std::string, std::set<std::string>> A map of allergens to their possible ingredients (not concrete)
 */
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

/**
 * Given a map of the known allergen mappings, generate the canonical name
 * @param mappedIngredients The mapped ingredients
 * @return std::string The canonical name of the food
 */
std::string generateCanonicalName(const std::map<std::string, std::string> &mappedIngredients) {
	std::vector<std::string> sortedAllergens;
	std::transform(
		mappedIngredients.cbegin(),
		mappedIngredients.cend(),
		std::back_inserter(sortedAllergens),
		[](const auto &entry) { return entry.first; });

	std::sort(sortedAllergens.begin(), sortedAllergens.end());

	std::vector<std::string> finalNameComponents;
	std::transform(
		sortedAllergens.cbegin(),
		sortedAllergens.cend(),
		std::back_inserter(finalNameComponents),
		[&mappedIngredients](const std::string &allergenName) { return mappedIngredients.at(allergenName); });

	return folly::join(",", finalNameComponents);
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

std::string part2(const std::vector<IngredientLineItem> &input) {
	auto knownCorrelations = correlateIngredients(input);
	// Prioritize the sets we know about based on the size of the containers they point to
	auto compareSizes = [&knownCorrelations](const std::string &allergen1, const std::string &allergen2) {
		const auto &correlationSet1 = knownCorrelations.at(allergen1);
		const auto &correlationSet2 = knownCorrelations.at(allergen2);
		return correlationSet1.size() < correlationSet2.size();
	};
	std::priority_queue<std::string, std::vector<std::string>, decltype(compareSizes)> toVisit(compareSizes);
	std::for_each(knownCorrelations.cbegin(), knownCorrelations.cend(), [&toVisit](const auto &entry) {
		toVisit.push(entry.first);
	});

	// toRevisit is used so we don't continually go back to the same set that happens to be the same size over and over
	// again
	std::vector<std::string> toRevisit;
	std::map<std::string, std::string> mappedIngredients;
	std::set<std::string> usedIngredients;
	int lastSize = 0;
	while (!toVisit.empty() || !toRevisit.empty()) {
		std::optional<std::string> allergenName;
		std::optional<std::reference_wrapper<std::set<std::string>>> correlationSet;
		// We can't get the allergen name if the visit set is empty
		if (!toVisit.empty()) {
			allergenName = toVisit.top();
			correlationSet = knownCorrelations.at(*allergenName);
		}
		if (toVisit.empty() || (correlationSet->get().size() > lastSize && !toRevisit.empty())) {
			// priority_queue does not provide a ranged insert - do this by hand.
			std::for_each(toRevisit.begin(), toRevisit.end(), [&toVisit](const std::string set) { toVisit.push(set); });
			toRevisit.clear();
			continue;
		}

		// This pop is safe - we know that the set must have at least one element at this point
		toVisit.pop();
		lastSize = correlationSet->get().size();
		std::set<std::string> ingredientSet;
		std::set_difference(
			correlationSet->get().cbegin(),
			correlationSet->get().cend(),
			usedIngredients.cbegin(),
			usedIngredients.cend(),
			std::inserter(ingredientSet, ingredientSet.end()));

		// If there's only one element, we know what the allergen correlates to, so we're done
		if (ingredientSet.size() == 1) {
			std::string ingredientName = *ingredientSet.begin();
			usedIngredients.insert(ingredientName);
			mappedIngredients.emplace(*allergenName, ingredientName);
		} else {
			// If there's more than one, save the updated set, and move on
			knownCorrelations[*allergenName] = ingredientSet;
			toRevisit.push_back(*allergenName);
		}
	}

	return generateCanonicalName(mappedIngredients);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = parseInput(input);

	std::cout << part1(parsedInput) << std::endl;
	std::cout << part2(parsedInput) << std::endl;
}

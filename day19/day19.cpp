#include <folly/Format.h>
#include <folly/String.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <string>
#include <vector>

constexpr auto RULE_DELIM = ":";
constexpr auto ALTERNATING_DELIM = " | ";
constexpr int NUM_RULE_11_CYCLES = 8;

class GrammarEntry;
using MultiGrammarEntry = std::vector<GrammarEntry>;
using AlternatableMultiGrammarEntry = std::vector<MultiGrammarEntry>;

/**
 * An entry in the grammar. This can either be a layer of indirection to another entry (a lookup) or a grammar rule
 * itself.
 */
class GrammarEntry {
 public:
	GrammarEntry(int index) : index(index), isThisALookup(true) {
	}

	GrammarEntry(char value) : value(value), isThisALookup(false) {
	}

	/**
	 * Checks whether or not this grammar rule is an indirection to another
	 *
	 * @return bool Whether or not thisg rammar ule is an indirection to another.
	 */
	bool isLookup() const {
		return this->isThisALookup;
	}

	/**
	 * @return int The index of the rule that this looks up
	 */
	int getIndex() const {
		if (!this->isLookup()) {
			throw "Cannot get the index of a non-lookup";
		}

		return this->index;
	}

	/**
	 * Checks the value of this grammar rule, if it is not a lookup.
	 *
	 * @return The character that this grammar rule represents.
	 */
	char getValue() const {
		if (isThisALookup) {
			throw "Cannot get the value of a lookup";
		}

		return this->value;
	}

 private:
	int index;
	char value;
	bool isThisALookup;
};

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
 * Split the input into grammar and test strings
 *
 * @param input The puzzle input.
 *
 * @return std::pair<std::vector<std::string>, std::vector<std::string>> A pair of the puzzle grammar and the puzzle
 * test strings
 */
std::pair<std::vector<std::string>, std::vector<std::string>> splitInput(const std::vector<std::string> &input) {
	auto emptyLine = std::find(input.cbegin(), input.cend(), "");
	auto beforeEmptyLine = std::vector<std::string>(input.cbegin(), emptyLine);
	auto afterEmptyLine = std::vector<std::string>(emptyLine + 1, input.cend());
	return std::pair<std::vector<std::string>, std::vector<std::string>>(
		std::move(beforeEmptyLine), std::move(afterEmptyLine));
}

/**
 * Convert a single pattern to a grammar entry
 *
 * @param rawPattern The pattern from the entry to parse.
 *
 * @return MultiGrammarEntry The grammar entry to parse, which is a vector of single entries, each element
 * representating an alternation.
 */
MultiGrammarEntry parseSinglePattern(const std::string &rawPattern) {
	std::vector<std::string> rawPatternComponents;
	folly::split(" ", rawPattern, rawPatternComponents);
	MultiGrammarEntry patternComponents;
	std::transform(
		rawPatternComponents.cbegin(),
		rawPatternComponents.cend(),
		std::back_inserter(patternComponents),
		[](const std::string &rawComponent) {
			if (rawComponent.at(0) == '"' && rawComponent.at(rawComponent.size() - 1) == '"') {
				std::string component = rawComponent.substr(1, rawComponent.size() - 2);
				if (component.size() != 1) {
					throw std::invalid_argument("Base rule must be 1 char");
				}

				return GrammarEntry(component.at(0));
			} else {
				return GrammarEntry(std::stoi(rawComponent));
			}
		});

	return patternComponents;
}

/**
 * Parse the grammar part of the puzzle into a map of grammar entries.
 *
 * @param patterns The pattern from the entry to parse.
 *
 * @return std::unordered_multimap<int, MultiGrammarEntry> A map of grammar rule indices to grammar entries. Each
 * element of the grammar entry vectors are alternations.
 */
std::unordered_multimap<int, MultiGrammarEntry> parseGrammar(const std::vector<std::string> &patterns) {
	std::unordered_multimap<int, MultiGrammarEntry> grammar;
	for (const std::string &patternLine : patterns) {
		auto colonIndex = patternLine.find(RULE_DELIM);
		std::string rawIndex = patternLine.substr(0, colonIndex);
		int patternIndex = std::stoi(rawIndex);
		std::vector<std::string> rawAlternations;
		// An extra +1 on the colon index to get rid of the space after the colon
		folly::split(ALTERNATING_DELIM, patternLine.substr(colonIndex + 2), rawAlternations);

		std::transform(
			rawAlternations.cbegin(),
			rawAlternations.cend(),
			std::inserter(grammar, grammar.end()),
			[patternIndex](const std::string &pattern) {
				return std::make_pair(patternIndex, parseSinglePattern(pattern));
			});
	}

	return grammar;
}

/**
 * Convert a grammar to a regular expression
 *
 * @param grammar The grammar for the puzzle
 * @param rule The grammar rule to start the search at
 *
 * @return A regular expression for the puzzle input.
 */
std::string convertToRegularExpression(const std::unordered_multimap<int, MultiGrammarEntry> &grammar, int rule = 0) {
	std::vector<std::string> expressions;
	auto ruleIterators = grammar.equal_range(rule);
	std::vector<std::pair<int, MultiGrammarEntry>> entries(ruleIterators.first, ruleIterators.second);
	std::transform(
		entries.cbegin(),
		entries.cend(),
		std::back_inserter(expressions),
		[&grammar, rule](const std::pair<int, MultiGrammarEntry> &entry) {
			MultiGrammarEntry alternative = entry.second;
			std::string prefix;
			std::string expression;
			std::string suffixPrefix;
			std::string suffix;
			bool isNonRegularCycle = false;
			for (const GrammarEntry &grammarEntry : alternative) {
				if (!grammarEntry.isLookup()) {
					expression += grammarEntry.getValue();
				} else if (
					grammarEntry.getIndex() == rule && alternative.back().isLookup() &&
					rule == alternative.back().getIndex()) {
					expression = folly::format("(?:{})+?", expression).str();
				} else if (grammarEntry.getIndex() == rule) {
					isNonRegularCycle = true;
					// This grossness will be resolved when using a format string. We want to be able to replace this
					// with a number later.
					suffixPrefix = folly::format("(?:{}){{{{{{}}}}}}(?:", expression).str();
					expression.clear();
					suffix = "){{{}}}";
				} else {
					expression += convertToRegularExpression(grammar, grammarEntry.getIndex());
				}
			}

			if (!isNonRegularCycle) {
				return expression;
			} else {
				std::vector<std::string> fullSuffixElements;
				// This is awful, but basically, checking for 31 11 42 is not posisble with a regular language, so we
				// must check the possibilities of 1 of each, 2 of each, ... the value of NUM_RULE_11_CYCLES was picked
				// arbitrarily because it works with my input.
				for (int i = 1; i < NUM_RULE_11_CYCLES; i++) {
					fullSuffixElements.push_back(
						folly::format(suffixPrefix, i).str() + expression + folly::format(suffix, i).str());
				}

				return "(" + folly::join("|", fullSuffixElements) + ")";
			}
		});

	if (expressions.size() == 1) {
		return expressions.at(0);
	} else {
		// Create an alternation of all of the entries
		return folly::format("(?:{})", folly::join("|", expressions)).str();
	}
}

/**
 * Get the number of times the grammar matches the test strings
 *
 * @param grammar The grammar to check against
 * @param testStrings The strings to test if they match
 *
 * @returns int The number of test strings that match the grammar
 */
int getNumberOfMatches(
	const std::unordered_multimap<int, MultiGrammarEntry> &grammar, const std::vector<std::string> &testStrings) {
	std::regex inputRegex(convertToRegularExpression(grammar));
	return std::count_if(testStrings.cbegin(), testStrings.cend(), [&inputRegex](const std::string &testString) {
		std::smatch matches;
		return std::regex_match(testString, matches, inputRegex);
	});
}

int part1(const std::vector<std::string> &patterns, const std::vector<std::string> &testStrings) {
	std::unordered_multimap<int, MultiGrammarEntry> grammar = parseGrammar(patterns);
	return getNumberOfMatches(grammar, testStrings);
}

int part2(const std::vector<std::string> &patterns, const std::vector<std::string> &testStrings) {
	std::unordered_multimap<int, MultiGrammarEntry> grammar = parseGrammar(patterns);
	grammar.erase(8);
	grammar.erase(11);
	grammar.emplace(8, MultiGrammarEntry{GrammarEntry(42)});
	grammar.emplace(8, MultiGrammarEntry{GrammarEntry(42), GrammarEntry(8)});
	grammar.emplace(11, MultiGrammarEntry{GrammarEntry(42), GrammarEntry(31)});
	grammar.emplace(11, MultiGrammarEntry{GrammarEntry(42), GrammarEntry(11), GrammarEntry(31)});
	return getNumberOfMatches(grammar, testStrings);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = splitInput(input);

	std::cout << part1(parsedInput.first, parsedInput.second) << std::endl;
	std::cout << part2(parsedInput.first, parsedInput.second) << std::endl;
}

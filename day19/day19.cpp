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

constexpr auto ALTERNATING_DELIM = " | ";
class GrammarEntry;
using MultiGrammarEntry = std::vector<GrammarEntry>;
using AlternatableMultiGrammarEntry = std::vector<MultiGrammarEntry>;

class GrammarEntry {
 public:
	GrammarEntry(int index) : index(index), isThisALookup(true) {
	}

	GrammarEntry(char value) : value(value), isThisALookup(false) {
	}

	bool isLookup() const {
		return this->isThisALookup;
	}

	int getIndex() const {
		if (!isThisALookup) {
			throw "Cannot get the index of a non-lookup";
		}

		return this->index;
	}

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

std::pair<std::vector<std::string>, std::vector<std::string>> splitInput(const std::vector<std::string> &input) {
	auto emptyLine = std::find(input.cbegin(), input.cend(), "");
	auto beforeEmptyLine = std::vector<std::string>(input.cbegin(), emptyLine);
	auto afterEmptyLine = std::vector<std::string>(emptyLine + 1, input.cend());
	return std::pair<std::vector<std::string>, std::vector<std::string>>(
		std::move(beforeEmptyLine), std::move(afterEmptyLine));
}

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

std::unordered_multimap<int, MultiGrammarEntry> parseGrammar(const std::vector<std::string> &patterns) {
	std::unordered_multimap<int, MultiGrammarEntry> grammar;
	for (const std::string &patternLine : patterns) {
		auto colonIndex = patternLine.find(":");
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

std::string convertToRegularExpression(const std::unordered_multimap<int, MultiGrammarEntry> &grammar, int rule) {
	std::vector<std::string> expressions;
	auto ruleIterators = grammar.equal_range(rule);
	std::transform(
		ruleIterators.first,
		ruleIterators.second,
		std::back_inserter(expressions),
		[&grammar](const std::pair<int, MultiGrammarEntry> &entry) {
			MultiGrammarEntry alternative = entry.second;
			std::string expression;
			for (const GrammarEntry &grammarEntry : alternative) {
				if (grammarEntry.isLookup()) {
					expression += convertToRegularExpression(grammar, grammarEntry.getIndex());
				} else {
					expression += grammarEntry.getValue();
				}
			}

			return expression;
		});

	if (expressions.size() == 1) {
		return expressions.at(0);
	} else {
		return folly::format("({})", folly::join("|", expressions)).str();
	}
}

std::regex convertToRegularExpression(const std::unordered_multimap<int, MultiGrammarEntry> &grammar) {
	auto rawRegularExpression = convertToRegularExpression(grammar, 0);
	return std::regex(rawRegularExpression);
}

int part1(const std::vector<std::string> &patterns, const std::vector<std::string> &testStrings) {
	std::unordered_multimap<int, MultiGrammarEntry> grammar = parseGrammar(patterns);
	std::regex inputRegex = convertToRegularExpression(grammar);
	return std::count_if(testStrings.cbegin(), testStrings.cend(), [&inputRegex](const std::string &testString) {
		return std::regex_match(testString, inputRegex);
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = splitInput(input);

	std::cout << part1(parsedInput.first, parsedInput.second) << std::endl;
}

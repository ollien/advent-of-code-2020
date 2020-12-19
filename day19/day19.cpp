#include <folly/String.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
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

std::unordered_map<int, AlternatableMultiGrammarEntry> parseGrammar(const std::vector<std::string> &patterns) {
	std::unordered_map<int, AlternatableMultiGrammarEntry> grammar;
	std::transform(
		patterns.cbegin(), patterns.cend(), std::inserter(grammar, grammar.end()), [](const std::string &patternLine) {
			auto colonIndex = patternLine.find(":");
			std::string rawIndex = patternLine.substr(0, colonIndex);
			int patternIndex = std::stoi(rawIndex);
			std::vector<std::string> rawAlternations;
			// An extra +1 on the colon index to get rid of the space after the colon
			folly::split(ALTERNATING_DELIM, patternLine.substr(colonIndex + 2), rawAlternations);

			AlternatableMultiGrammarEntry entries;
			std::transform(
				rawAlternations.cbegin(), rawAlternations.cend(), std::back_inserter(entries), parseSinglePattern);

			return std::pair<int, AlternatableMultiGrammarEntry>(patternIndex, entries);
		});

	return grammar;
}

int matchesHowManyChars(
	const std::unordered_map<int, AlternatableMultiGrammarEntry> &grammar, const std::string_view toParse, int rule = 0,
	int depth = 0) {
	auto indentation = std::string(depth, '|');
	std::cout << indentation << "Working on " << toParse << std::endl;
	AlternatableMultiGrammarEntry ruleAlternations = grammar.at(rule);
	std::optional<std::pair<MultiGrammarEntry, int>> best;
	for (const MultiGrammarEntry &alternation : ruleAlternations) {
		int numMatched = 0;
		int i = 0;
		for (auto entryIt = alternation.cbegin(); entryIt != alternation.cend(); (++entryIt, i++)) {
			GrammarEntry entry = *entryIt;
			int ruleMatchCount;
			if (entry.isLookup()) {
				std::cout << indentation << "Checking rule " << entry.getIndex() << std::endl;
				ruleMatchCount = matchesHowManyChars(grammar, toParse.substr(numMatched), entry.getIndex(), depth + 1);
			} else {
				std::cout << indentation << "Checking rule " << entry.getValue() << std::endl;
				char toMatch = toParse.at(i);
				char ruleValue = entry.getValue();
				ruleMatchCount = (toMatch == ruleValue);
			}

			std::cout << indentation << "Result: " << numMatched << std::endl;

			if (!ruleMatchCount) {
				break;
			}

			numMatched += ruleMatchCount;
		}

		std::cout << indentation << "Full result: " << numMatched << std::endl;
		if (!best || numMatched > best->second) {
			best = std::pair<MultiGrammarEntry, int>(alternation, numMatched);
		}
	}

	if (!best) {
		return 0;
	}

	return best->second;
}

int part1(const std::vector<std::string> &patterns, const std::vector<std::string> &testStrings) {
	std::unordered_map<int, AlternatableMultiGrammarEntry> grammar = parseGrammar(patterns);
	return std::count_if(testStrings.cbegin(), testStrings.cend(), [&grammar](const std::string &testString) {
		return matchesHowManyChars(grammar, testString) == testString.length();
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

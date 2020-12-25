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
 * Split the input between each player's decks
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

template <typename Iterator, typename IntContainer>
void containerStoi(Iterator begin, Iterator end, IntContainer &out) {
	std::transform(begin, end, std::back_inserter(out), [](const std::string &item) { return std::stoi(item); });
}

std::pair<std::vector<int>, std::vector<int>> parseDecks(const std::vector<std::string> &input) {
	std::pair<std::vector<std::string>, std::vector<std::string>> inputComponents = splitInput(input);
	std::pair<std::vector<int>, std::vector<int>> decks;
	containerStoi(inputComponents.first.cbegin() + 1, inputComponents.first.cend(), decks.first);
	containerStoi(inputComponents.second.cbegin() + 1, inputComponents.second.cend(), decks.second);

	return decks;
}

std::vector<int> playBasicGame(const std::pair<std::vector<int>, std::vector<int>> &initialDecks) {
	std::pair<std::deque<int>, std::deque<int>> decks{
		std::deque<int>(initialDecks.first.cbegin(), initialDecks.first.cend()),
		std::deque<int>(initialDecks.second.cbegin(), initialDecks.second.cend())};
	int i = 0;
	while (!decks.first.empty() && !decks.second.empty()) {
		i++;
		int player1Card = decks.first.front();
		int player2Card = decks.second.front();
		decks.first.pop_front();
		decks.second.pop_front();
		if (player1Card > player2Card) {
			decks.first.push_back(player1Card);
			decks.first.push_back(player2Card);
		} else {
			decks.second.push_back(player2Card);
			decks.second.push_back(player1Card);
		}
	}

	std::deque<int> &winnerDeck = decks.first.empty() ? decks.second : decks.first;

	return std::vector<int>(winnerDeck.cbegin(), winnerDeck.cend());
}

int part1(const std::pair<std::vector<int>, std::vector<int>> &decks) {
	std::vector<int> winnerDeck = playBasicGame(decks);
	int i = 1;
	int total = 0;
	for (auto it = winnerDeck.crbegin(); it != winnerDeck.crend(); (++it, i++)) {
		int value = *it;
		total += i * value;
	}

	return total;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto decks = parseDecks(input);

	std::cout << part1(decks) << std::endl;
}

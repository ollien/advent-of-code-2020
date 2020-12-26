#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <vector>

enum Player { PLAYER1, PLAYER2 };

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

/**
 * Perform std::stoi on every element in the container, pushing the items to the output iterator.
 * @tparam Iterator The iterator for the input container
 * @tparam IntContainer The container to output to
 * @param begin The start of the range
 * @param end The end of the range
 * @param out The output iterator
 */
template <typename InputIterator, typename OutputIterator>
void containerStoi(InputIterator begin, InputIterator end, OutputIterator out) {
	std::transform(begin, end, out, [](const std::string &item) { return std::stoi(item); });
}

/**
 * Parse the decks from the puzzle input
 * @param input The puzzle input
 * @return std::pair<std::vector<int>, std::vector<int>> Both decks to play the game (player 1, player 2)
 */
std::pair<std::vector<int>, std::vector<int>> parseDecks(const std::vector<std::string> &input) {
	std::pair<std::vector<std::string>, std::vector<std::string>> inputComponents = splitInput(input);
	std::pair<std::vector<int>, std::vector<int>> decks;
	containerStoi(inputComponents.first.cbegin() + 1, inputComponents.first.cend(), std::back_inserter(decks.first));
	containerStoi(inputComponents.second.cbegin() + 1, inputComponents.second.cend(), std::back_inserter(decks.second));

	return decks;
}

/**
 * Calculate the score for a game
 * @tparam ReverseIterator A reverse iterator for the winning deck. Could technically be any input iterator but the
 * score calculation requires going backwards
 * @param begin The start of the deck
 * @param end The end of the deck
 * @return int The game score
 */
template <typename ReverseIterator>
int calculateScore(ReverseIterator begin, ReverseIterator end) {
	int i = 1;
	int total = 0;
	for (auto it = begin; it != end; (++it, i++)) {
		int value = *it;
		total += i * value;
	}

	return total;
}

int part1(const std::pair<std::vector<int>, std::vector<int>> &initialDecks) {
	std::pair<std::deque<int>, std::deque<int>> decks{
		std::deque<int>(initialDecks.first.cbegin(), initialDecks.first.cend()),
		std::deque<int>(initialDecks.second.cbegin(), initialDecks.second.cend())};
	while (!decks.first.empty() && !decks.second.empty()) {
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

	return calculateScore(winnerDeck.crbegin(), winnerDeck.crend());
}

int part2(const std::pair<std::vector<int>, std::vector<int>> &initialDecks) {
	using DeckPair = std::pair<std::deque<int>, std::deque<int>>;
	std::function<std::pair<Player, DeckPair>(DeckPair &)> playGame =
		[&playGame](DeckPair &decks) -> std::pair<Player, std::pair<std::deque<int>, std::deque<int>>> {
		// Holds game states that have already been used, preventing their resuse within a single sub-game
		std::set<DeckPair> usedDecks;
		while (!decks.first.empty() && !decks.second.empty()) {
			if (usedDecks.find(decks) != usedDecks.end()) {
				return std::make_pair(PLAYER1, decks);
			}

			usedDecks.insert(decks);
			int player1Card = decks.first.front();
			int player2Card = decks.second.front();
			Player roundWinner;
			if (player1Card < decks.first.size() && player2Card < decks.second.size()) {
				// Must be done after the size check...
				decks.first.pop_front();
				decks.second.pop_front();
				DeckPair truncatedDecks;
				std::copy_n(decks.first.cbegin(), player1Card, std::back_inserter(truncatedDecks.first));
				std::copy_n(decks.second.cbegin(), player2Card, std::back_inserter(truncatedDecks.second));
				roundWinner = playGame(truncatedDecks).first;
			} else {
				roundWinner = player1Card > player2Card ? PLAYER1 : PLAYER2;
				decks.first.pop_front();
				decks.second.pop_front();
			}

			if (roundWinner == PLAYER1) {
				decks.first.push_back(player1Card);
				decks.first.push_back(player2Card);
			} else {
				decks.second.push_back(player2Card);
				decks.second.push_back(player1Card);
			}
		}

		return std::make_pair(decks.first.empty() ? PLAYER2 : PLAYER1, decks);
	};

	std::pair<std::deque<int>, std::deque<int>> decks{
		std::deque<int>(initialDecks.first.cbegin(), initialDecks.first.cend()),
		std::deque<int>(initialDecks.second.cbegin(), initialDecks.second.cend())};

	auto gameResult = playGame(decks);
	auto winningDeck = gameResult.first == PLAYER1 ? gameResult.second.first : gameResult.second.second;

	return calculateScore(winningDeck.crbegin(), winningDeck.crend());
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto decks = parseDecks(input);

	std::cout << part1(decks) << std::endl;
	std::cout << part2(decks) << std::endl;
}

#include <folly/String.h>

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

constexpr char ALIVE_CHAR = '#';
constexpr char DEAD_CHAR = '.';
constexpr int CYCLE_COUNT = 6;

enum CellState { ALIVE, DEAD };

using Board = std::map<std::tuple<int, int, int>, CellState>;

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

Board parseBoard(const std::vector<std::string> &input) {
	Board board;
	int rowCursor = 0;
	for (auto rowIterator = input.cbegin(); rowIterator != input.cend(); (rowCursor++, ++rowIterator)) {
		auto &line = *rowIterator;
		int colCursor = 0;
		for (auto colIterator = line.cbegin(); colIterator != line.cend(); (colCursor++, ++colIterator)) {
			CellState state = *colIterator == ALIVE_CHAR ? ALIVE : DEAD;
			std::tuple<int, int, int> position(rowCursor, colCursor, 0);
			board.emplace(std::move(position), std::move(state));
		}
	}

	return board;
}

int getNumAdjacentLiveNeighbors(const Board &board, int row, int col, int depth) {
	int count = 0;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			for (int dDepth = -1; dDepth <= 1; dDepth++) {
				if (dRow == 0 && dCol == 0 && dDepth == 0) {
					continue;
				}

				std::tuple<int, int, int> position(row + dRow, col + dCol, depth + dDepth);
				if (board.find(position) == board.end()) {
					continue;
				}

				count += (board.at(position) == ALIVE);
			}
		}
	}

	return count;
}

template <int componentIndex>
std::pair<int, int> getMinMax(const Board &board) {
	std::vector<std::pair<std::tuple<int, int, int>, CellState>> alivePositions;
	std::copy_if(
		board.cbegin(),
		board.cend(),
		std::back_inserter(alivePositions),
		[](const std::pair<std::tuple<int, int, int>, CellState> &item) { return item.second == ALIVE; });

	auto elements = std::minmax_element(
		alivePositions.cbegin(),
		alivePositions.cend(),
		[](const std::pair<std::tuple<int, int, int>, CellState> &item,
		   const std::pair<std::tuple<int, int, int>, CellState> &item2) {
			return std::get<componentIndex>(item.first) < std::get<componentIndex>(item2.first);
		});
	int minComponent = std::get<componentIndex>(elements.first->first);
	int maxComponent = std::get<componentIndex>(elements.second->first);

	return std::pair<int, int>(minComponent, maxComponent);
}

std::pair<std::tuple<int, int, int>, std::tuple<int, int, int>> getRangeOnEachAxis(const Board &board) {
	std::pair<int, int> rowRange = getMinMax<0>(board);
	std::pair<int, int> colRange = getMinMax<1>(board);
	std::pair<int, int> depthRange = getMinMax<2>(board);

	return std::pair<std::tuple<int, int, int>, std::tuple<int, int, int>>(
		std::tuple<int, int, int>(rowRange.first, colRange.first, depthRange.first),
		std::tuple<int, int, int>(rowRange.second, colRange.second, depthRange.second));
}

template <typename Key, typename Val>
Val getOrDefault(const std::map<Key, Val> &map, Val defaultValue, Key key) {
	if (map.find(key) == map.end()) {
		return defaultValue;
	}

	return map.at(key);
}

void printBoard(const Board &board) {
	auto ranges = getRangeOnEachAxis(board);
	for (int depth = std::get<2>(ranges.first); depth <= std::get<2>(ranges.second); depth++) {
		std::cout << "Depth z=" << depth << std::endl;
		for (int row = std::get<0>(ranges.first); row <= std::get<0>(ranges.second); row++) {
			for (int col = std::get<1>(ranges.first); col <= std::get<1>(ranges.second); col++) {
				std::tuple<int, int, int> position(row, col, depth);
				CellState state = getOrDefault(board, DEAD, position);
				std::cout << ((state == ALIVE) ? ALIVE_CHAR : DEAD_CHAR);
			}
			std::cout << std::endl;
		}
	}
	std::cout << std::endl;
}

int part1(const std::vector<std::string> &input) {
	Board board = parseBoard(input);
	Board nextBoard = board;
	for (int i = 0; i < CYCLE_COUNT; i++) {
		printBoard(board);
		auto ranges = getRangeOnEachAxis(board);
		for (int row = std::get<0>(ranges.first) - 1; row <= std::get<0>(ranges.second) + 1; row++) {
			for (int col = std::get<1>(ranges.first) - 1; col <= std::get<1>(ranges.second) + 1; col++) {
				for (int depth = std::get<2>(ranges.first) - 1; depth <= std::get<2>(ranges.second) + 1; depth++) {
					int aliveNeighbors = getNumAdjacentLiveNeighbors(board, row, col, depth);
					std::tuple<int, int, int> position(row, col, depth);
					CellState state = getOrDefault(board, DEAD, position);
					if (aliveNeighbors != 2 && aliveNeighbors != 3) {
						state = DEAD;
					} else if (state == DEAD && aliveNeighbors == 3) {
						state = ALIVE;
					}

					nextBoard[position] = state;
				}
			}
		}
		std::swap(nextBoard, board);
	}

	return std::count_if(board.cbegin(), board.cend(), [](const std::pair<std::tuple<int, int, int>, CellState> &item) {
		return item.second == ALIVE;
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << part1(input) << std::endl;
}

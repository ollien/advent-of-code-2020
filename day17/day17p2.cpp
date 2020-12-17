#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "common.hpp"

using Position = std::tuple<int, int, int, int>;
using Board = std::map<Position, CellState>;

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
			Position position(rowCursor, colCursor, 0, 0);
			board.emplace(std::move(position), std::move(state));
		}
	}

	return board;
}

int getNumAdjacentLiveNeighbors(const Board &board, int row, int col, int depth, int w) {
	int count = 0;
	for (int dRow = -1; dRow <= 1; dRow++) {
		for (int dCol = -1; dCol <= 1; dCol++) {
			for (int dDepth = -1; dDepth <= 1; dDepth++) {
				for (int dW = -1; dW <= 1; dW++) {
					if (dRow == 0 && dCol == 0 && dDepth == 0 && dW == 0) {
						continue;
					}

					Position position(row + dRow, col + dCol, depth + dDepth, w + dW);
					if (board.find(position) == board.end()) {
						continue;
					}

					count += (board.at(position) == ALIVE);
				}
			}
		}
	}

	return count;
}

/**
 * Get the range alive positions of each component
 * @param board The board to check
 * @return std::pair<Position, Position> The min and max of each component on the board
 */
std::pair<Position, Position> getRangeOnEachAxis(const Board &board) {
	std::pair<int, int> rowRange = getMinMaxAlivePositionsForComponent<Position, 0>(board);
	std::pair<int, int> colRange = getMinMaxAlivePositionsForComponent<Position, 1>(board);
	std::pair<int, int> depthRange = getMinMaxAlivePositionsForComponent<Position, 2>(board);
	std::pair<int, int> wRange = getMinMaxAlivePositionsForComponent<Position, 3>(board);

	return std::pair<Position, Position>(
		Position(rowRange.first, colRange.first, depthRange.first, wRange.first),
		Position(rowRange.second, colRange.second, depthRange.second, wRange.second));
}

int run(const std::vector<std::string> &input) {
	Board board = parseBoard(input);
	Board nextBoard = board;
	for (int i = 0; i < CYCLE_COUNT; i++) {
		auto ranges = getRangeOnEachAxis(board);
		for (int row = std::get<0>(ranges.first) - 1; row <= std::get<0>(ranges.second) + 1; row++) {
			for (int col = std::get<1>(ranges.first) - 1; col <= std::get<1>(ranges.second) + 1; col++) {
				for (int depth = std::get<2>(ranges.first) - 1; depth <= std::get<2>(ranges.second) + 1; depth++) {
					for (int w = std::get<3>(ranges.first) - 1; w <= std::get<3>(ranges.second) + 1; w++) {
						int aliveNeighbors = getNumAdjacentLiveNeighbors(board, row, col, depth, w);
						Position position(row, col, depth, w);
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
		}
		std::swap(nextBoard, board);
	}

	return std::count_if(
		board.cbegin(), board.cend(), [](const std::pair<Position, CellState> &item) { return item.second == ALIVE; });
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);

	std::cout << run(input) << std::endl;
}

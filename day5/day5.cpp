#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

// The maximum row of the plane
constexpr int MAX_ROW = 127;
constexpr int MAX_COL = 7;
constexpr char FRONT_CHAR = 'F';
constexpr char BACK_CHAR = 'B';
constexpr char RIGHT_CHAR = 'R';
constexpr char LEFT_CHAR = 'L';

std::vector<std::string> read_input(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

int getPosFromSpec(std::string_view view, char bottomHalfChar, char topHalfChar, int initMax) {
	int max = initMax;
	int min = 0;
	int stringCursor = 0;
	for (char candidate : view) {
		// std::cout << min << " " << max << std::endl;
		// std::cout << candidate << std::endl;
		if (candidate == topHalfChar) {
			max = (max + min) / 2;
		} else if (candidate == bottomHalfChar) {
			min += (max - min) / 2 + 1;
		}

		stringCursor++;
	}

	return min;
}

int getColumnSplitPoint(std::string_view view) {
	int rightPosition = view.find(RIGHT_CHAR);
	int leftPosition = view.find(LEFT_CHAR);
	if (rightPosition == std::string_view::npos) {
		return leftPosition;
	} else if (leftPosition == std::string_view::npos) {
		return rightPosition;
	}

	return std::min(rightPosition, leftPosition);
}

int parseSeatID(std::string_view view) {
	int columnSplitPoint = getColumnSplitPoint(view);
	std::string_view rowSpec = view.substr(0, columnSplitPoint);
	std::string_view colSpec = view.substr(columnSplitPoint);
	// std::cout << rowSpec << " " << colSpec << std::endl;

	int rowId = getPosFromSpec(rowSpec, BACK_CHAR, FRONT_CHAR, MAX_ROW);
	int colId = getPosFromSpec(colSpec, RIGHT_CHAR, LEFT_CHAR, MAX_COL);

	// std::cout << rowId << " " << colId << std::endl;
	return rowId * (MAX_COL + 1) + colId;
}

int part1(const std::vector<std::string> &input) {
	return std::accumulate(input.cbegin(), input.cend(), 0, [](int max, const std::string &row) {
		int seatId = parseSeatID(row);
		return std::max(seatId, max);
	});
}

int part2(const std::vector<std::string> &input) {
	std::vector<int> ids;
	std::transform(input.cbegin(), input.cend(), std::inserter(ids, ids.begin()), [](const std::string &input) {
		return parseSeatID(input);
	});
	std::sort(ids.begin(), ids.end());

	int lastId = ids.at(0);
	for (auto it = std::next(ids.begin()); it != ids.end(); it++) {
		if (*it != lastId + 1) {
			return lastId + 1;
		}
		lastId = *it;
	}

	throw std::invalid_argument("No solution in given input");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = read_input(argv[1]);
	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
}

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

/**
 * Get the position of a row or a column from a string spec.
 * @param spec The specification of the row or the column for the problem
 * @param bottomHalfChar The character that specifies taking the bottom half of the row/column
 * @param topHalfChar The character that specifies using the top half of the row/column
 * @param initMax The initial maximum value for the unit being checked (row or column).
 * @return int The row/column position from the spec.
 */
int getPosFromSpec(std::string_view spec, char bottomHalfChar, char topHalfChar, int initMax) {
	int max = initMax;
	int min = 0;
	for (char candidate : spec) {
		if (candidate == topHalfChar) {
			max = (max + min) / 2;
		} else if (candidate == bottomHalfChar) {
			min += (max - min) / 2 + 1;
		}
	}

	return min;
}

/**
 * Get the point at which the specification splits between row and column (i.e. from F/B and L/R)
 * @param seatSpec The specification for the seat
 * @return int The index at which rows stop being specified
 */
int getColumnSplitPoint(std::string_view seatSpec) {
	int rightPosition = seatSpec.find(RIGHT_CHAR);
	int leftPosition = seatSpec.find(LEFT_CHAR);
	if (rightPosition == std::string_view::npos) {
		return leftPosition;
	} else if (leftPosition == std::string_view::npos) {
		return rightPosition;
	}

	return std::min(rightPosition, leftPosition);
}

/**
 * Get the seat ID from the seat specification
 * @param seatSpec The specification for the seat specification
 * @return int The seatID given by seatSpec
 */
int parseSeatID(std::string_view seatSpec) {
	int columnSplitPoint = getColumnSplitPoint(seatSpec);
	std::string_view rowSpec = seatSpec.substr(0, columnSplitPoint);
	std::string_view colSpec = seatSpec.substr(columnSplitPoint);

	int rowId = getPosFromSpec(rowSpec, BACK_CHAR, FRONT_CHAR, MAX_ROW);
	int colId = getPosFromSpec(colSpec, RIGHT_CHAR, LEFT_CHAR, MAX_COL);

	return rowId * (MAX_COL + 1) + colId;
}

/**
 * Find the missing number in an iterator
 * @tparam Iter The iterator to check in
 * @param begin The start of the iterator
 * @param end The end of the iterator
 * @return std::iterator_traits<Iter>::value_type The missing number in the iterator
 * @throws invalid_argument if there is no missing number in the iterator
 */
template <typename Iter>
typename std::iterator_traits<Iter>::value_type findMissingNumber(Iter begin, Iter end) {
	auto lastNumber = *begin;
	for (Iter it = std::next(begin); it != end; it++) {
		if (*it != lastNumber + 1) {
			return lastNumber + 1;
		}

		lastNumber = *it;
	}

	throw std::invalid_argument("No missing number in iterator");
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

	return findMissingNumber(ids.begin(), ids.end());
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

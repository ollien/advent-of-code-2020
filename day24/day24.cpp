#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <tuple>

enum Direction { EAST, WEST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST, NORTH_EAST };

constexpr auto DIRECTION_PATTERN = R"(e|w|se|sw|nw|ne)";
constexpr int NUM_DAYS = 100;
const std::map<std::string, Direction> INPUT_TO_DIRECTION{
	std::make_pair("e", EAST),
	std::make_pair("w", WEST),
	std::make_pair("se", SOUTH_EAST),
	std::make_pair("sw", SOUTH_WEST),
	std::make_pair("nw", NORTH_WEST),
	std::make_pair("ne", NORTH_EAST),
};

// https://www.redblobgames.com/grids/hexagons/
// (He uses down to mean +y but I flipped that for my own sake).
const std::map<Direction, std::pair<int, int>> DIRECTION_TO_DELTA{
	std::make_pair(EAST, std::make_pair(-2, 0)),
	std::make_pair(WEST, std::make_pair(2, 0)),
	std::make_pair(SOUTH_EAST, std::make_pair(-1, -1)),
	std::make_pair(SOUTH_WEST, std::make_pair(1, -1)),
	std::make_pair(NORTH_WEST, std::make_pair(1, 1)),
	std::make_pair(NORTH_EAST, std::make_pair(-1, 1)),
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
 * Parse a single line of input
 * @param inputLine The line to parse
 * @return std::vector<Direction> A vector of the directions on each line.
 */
std::vector<Direction> parseInputLine(const std::string &inputLine) {
	std::regex directionPattern(DIRECTION_PATTERN);
	std::vector<Direction> res;
	std::transform(
		std::sregex_iterator(inputLine.cbegin(), inputLine.cend(), directionPattern),
		std::sregex_iterator(),
		std::back_inserter(res),
		[](const std::smatch &match) {
			std::string directionStr = match.str();
			return INPUT_TO_DIRECTION.at(directionStr);
		});

	return res;
}

/**
 * Parse the entire input
 * @param input The full puzzle input
 * @return std::vector<std::vector<Direction>> The parsed puzzle input, which is each line as a vector of directions.
 */
std::vector<std::vector<Direction>> parseInput(const std::vector<std::string> &input) {
	std::vector<std::vector<Direction>> res;
	std::transform(input.cbegin(), input.cend(), std::back_inserter(res), parseInputLine);

	return res;
}

/**
 * Get the final position from an iterator of directions
 * @tparam InputIterator An iterator of directions
 * @param start The start iterator
 * @param end The end iterator
 * @return std::pair<int, int> The final position from the sequence of directions
 */
template <typename InputIterator>
std::pair<int, int> getPositionFromDirections(InputIterator start, InputIterator end) {
	return std::accumulate(start, end, std::make_pair(0, 0), [](std::pair<int, int> position, const Direction &dir) {
		const std::pair<int, int> &delta = DIRECTION_TO_DELTA.at(dir);
		return std::make_pair(position.first + delta.first, position.second + delta.second);
	});
}

/**
 * Given the puzzle input, get the tiles that are flipped.
 * @param input The parsed puzzle input
 * @return std::map<std::pair<int, int>, bool> A map of positions to whether or not the tiles are flipped (i.e. black)
 */
std::map<std::pair<int, int>, bool> getFlippedTiles(const std::vector<std::vector<Direction>> &input) {
	std::map<std::pair<int, int>, bool> flipped;
	for (const std::vector<Direction> &sequence : input) {
		std::pair<int, int> pos = getPositionFromDirections(sequence.cbegin(), sequence.cend());
		flipped[pos] = !flipped[pos];
	}

	return flipped;
}

/**
 * Count the number of flipped (i.e. black) tiles
 * @param tiles The tiles on the board
 * @return int The number of flipped tiles
 */
int countFlippedTiles(std::map<std::pair<int, int>, bool> &tiles) {
	return std::count_if(tiles.cbegin(), tiles.cend(), [](auto entry) { return entry.second; });
}

/**
 * Get the neighbors of a given position
 * @param position The position to get the neighbor of
 * @return std::vector<std::pair<int, int>> The neighbors
 */
std::vector<std::pair<int, int>> getNeighbors(const std::pair<int, int> &position) {
	std::vector<std::pair<int, int>> res;

	std::transform(
		DIRECTION_TO_DELTA.cbegin(),
		DIRECTION_TO_DELTA.cend(),
		std::back_inserter(res),
		[&position](const auto &entry) {
			const std::pair<int, int> &delta = entry.second;
			return std::make_pair(position.first + delta.first, position.second + delta.second);
		});

	return res;
}

int part1(const std::vector<std::vector<Direction>> &input) {
	auto flipped = getFlippedTiles(input);

	return countFlippedTiles(flipped);
}

int part2(const std::vector<std::vector<Direction>> &input) {
	std::map<std::pair<int, int>, bool> flipped = getFlippedTiles(input);
	std::map<std::pair<int, int>, bool> next = getFlippedTiles(input);
	for (int i = 0; i < NUM_DAYS; i++) {
		// Ensure all entries have their neighbors in the map
		// If this is not true, the number of neighbors will not be run correctly.
		for (const auto &entry : next) {
			for (const auto &neighbor : getNeighbors(entry.first)) {
				// Emplace will not overwrite existing elements
				flipped.emplace(neighbor, false);
			}
		}

		next.clear();

		for (auto &entry : flipped) {
			auto neighbors = getNeighbors(entry.first);
			int numFlippedNeighbors =
				std::count_if(neighbors.cbegin(), neighbors.cend(), [&flipped](const std::pair<int, int> &pos) {
					auto posIt = flipped.find(pos);
					return posIt == flipped.end() ? false : posIt->second;
				});

			bool nextState = entry.second;
			if (entry.second && (numFlippedNeighbors == 0 || numFlippedNeighbors > 2)) {
				nextState = false;
				next.emplace(entry.first, false);
			} else if (!entry.second && numFlippedNeighbors == 2) {
				nextState = true;
			}

			next.emplace(entry.first, nextState);
		}

		std::swap(flipped, next);
	}

	return countFlippedTiles(flipped);
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

#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <tuple>

enum Direction { EAST, WEST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST, NORTH_EAST };

constexpr auto DIRECTION_PATTERN = R"(e|w|se|sw|nw|ne)";
const std::map<std::string, Direction> INPUT_TO_DIRECTION{
	std::make_pair("e", EAST),
	std::make_pair("w", WEST),
	std::make_pair("se", SOUTH_EAST),
	std::make_pair("sw", SOUTH_WEST),
	std::make_pair("nw", NORTH_WEST),
	std::make_pair("ne", NORTH_EAST),
};

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

std::vector<std::vector<Direction>> parseInput(const std::vector<std::string> &input) {
	std::vector<std::vector<Direction>> res;
	std::transform(input.cbegin(), input.cend(), std::back_inserter(res), parseInputLine);

	return res;
}

template <typename Iter>
std::pair<int, int> getPosition(Iter start, Iter end) {
	return std::accumulate(start, end, std::make_pair(0, 0), [](std::pair<int, int> position, const Direction &dir) {
		std::pair<int, int> delta = DIRECTION_TO_DELTA.at(dir);
		return std::make_pair(position.first + delta.first, position.second + delta.second);
	});
}

int part1(const std::vector<std::vector<Direction>> &input) {
	std::map<std::pair<int, int>, bool> flipped;
	for (const std::vector<Direction> &sequence : input) {
		std::pair<int, int> pos = getPosition(sequence.cbegin(), sequence.cend());
		flipped[pos] = !flipped[pos];
	}

	return std::count_if(flipped.cbegin(), flipped.cend(), [](auto entry) { return entry.second; });
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = parseInput(input);

	std::cout << part1(parsedInput) << std::endl;
}

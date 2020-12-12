#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

constexpr int NUM_DIRECTIONS = 4;
enum CardinalDirection { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

class Ship {
 public:
	Ship(CardinalDirection direction, std::pair<int, int> position) : direction(direction), position(position) {
	}

	const std::pair<int, int> &getPosition() const {
		return this->position;
	}

	CardinalDirection getDirection() const {
		return direction;
	}

	void setDirection(CardinalDirection direction) {
		this->direction = direction;
	}

	void move(int value) {
		this->move(value, this->direction);
	}

	void move(int value, CardinalDirection direction) {
		int delta = (direction == SOUTH || direction == EAST) ? -value : value;
		if (direction == NORTH || direction == SOUTH) {
			this->position.second += delta;
		} else {
			this->position.first += delta;
		}
	}

	void turnLeft(int deg) {
		this->addToDirection(3 * deg / 90);
	}

	void turnRight(int deg) {
		// Three lefts make a right - easier than dealing with negative mods
		this->addToDirection(deg / 90);
	}

 private:
	void addToDirection(int delta) {
		this->direction = static_cast<CardinalDirection>((this->direction + delta) % NUM_DIRECTIONS);
	}

	CardinalDirection direction;
	std::pair<int, int> position;
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

std::vector<std::pair<char, int>> parseInput(const std::vector<std::string> &input) {
	std::vector<std::pair<char, int>> parsedInput;
	parsedInput.reserve(input.size());
	for (std::string_view line : input) {
		char direction = line.at(0);
		auto rawMagnitude = line.substr(1);
		int magnitude;
		auto parseResult = std::from_chars(rawMagnitude.data(), rawMagnitude.data() + rawMagnitude.size(), magnitude);
		if (parseResult.ec == std::errc::invalid_argument) {
			throw std::invalid_argument("Invalid input");
		}

		parsedInput.push_back(std::pair<char, int>(direction, magnitude));
	}

	return parsedInput;
}

void makeMove(Ship &ship, const std::pair<char, int> &move) {
	char directive = move.first;
	int magnitude = move.second;
	switch (directive) {
		case 'L':
			ship.turnLeft(magnitude);
			break;
		case 'R':
			ship.turnRight(magnitude);
			break;
		case 'N':
			ship.move(magnitude, NORTH);
			break;
		case 'S':
			ship.move(magnitude, SOUTH);
			break;
		case 'E':
			ship.move(magnitude, EAST);
			break;
		case 'W':
			ship.move(magnitude, WEST);
			break;
		case 'F':
			ship.move(magnitude);
			break;
		default:
			throw std::invalid_argument("Invalid move direction");
	}
}

int part1(const std::vector<std::pair<char, int>> &input) {
	Ship ship(EAST, std::pair<int, int>(0, 0));
	for (auto &move : input) {
		makeMove(ship, move);
	}

	return abs(ship.getPosition().first) + abs(ship.getPosition().second);
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

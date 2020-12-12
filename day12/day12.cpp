#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

constexpr int NUM_DIRECTIONS = 4;
enum CardinalDirection { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

class MovablePoint {
 public:
	MovablePoint(CardinalDirection direction, std::pair<int, int> position) : direction(direction), position(position) {
	}

	const std::pair<int, int> &getPosition() const {
		return this->position;
	}

	void setPosition(const std::pair<int, int> &position) {
		this->position = position;
	}

	void setPosition(std::pair<int, int> &&position) {
		this->position = position;
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
		int delta = (direction == SOUTH || direction == WEST) ? -value : value;
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

	void rotatePosition(int degrees) {
		int x = this->position.first;
		int y = this->position.second;
		// This is an awful way of writing a rotation, but it basically just hacking in the three parts of the rotation
		// matrix I need
		auto stepAmount = degrees / abs(degrees / 90);
		if (abs(stepAmount) != 90) {
			throw std::invalid_argument("Invalid rotation amount");
		}
		for (int i = 0; i < abs(degrees / 90); i++) {
			int tmpX = x;
			x = -y;
			y = tmpX;
			if (stepAmount < 0) {
				x *= -1;
				y *= -1;
			}
		}

		this->position.first = x;
		this->position.second = y;
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

void moveShip(MovablePoint &ship, const std::pair<char, int> &move) {
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

void moveShipOrWaypoint(MovablePoint &ship, MovablePoint &waypoint, const std::pair<char, int> &move) {
	char directive = move.first;
	int magnitude = move.second;
	switch (directive) {
		case 'L':
			waypoint.rotatePosition(magnitude);
			break;
		case 'R':
			waypoint.rotatePosition(-magnitude);
			break;
		case 'N':
			waypoint.move(magnitude, NORTH);
			break;
		case 'S':
			waypoint.move(magnitude, SOUTH);
			break;
		case 'E':
			waypoint.move(magnitude, EAST);
			break;
		case 'W':
			waypoint.move(magnitude, WEST);
			break;
		case 'F': {
			auto shipPosition = ship.getPosition();
			shipPosition.first += waypoint.getPosition().first * magnitude;
			shipPosition.second += waypoint.getPosition().second * magnitude;
			ship.setPosition(std::move(shipPosition));
		} break;
		default:
			throw std::invalid_argument("Invalid move direction");
	}
}

int part1(const std::vector<std::pair<char, int>> &input) {
	MovablePoint ship(EAST, std::pair<int, int>(0, 0));
	for (auto &move : input) {
		moveShip(ship, move);
	}

	return abs(ship.getPosition().first) + abs(ship.getPosition().second);
}

int part2(const std::vector<std::pair<char, int>> &input) {
	MovablePoint ship(EAST, std::pair<int, int>(0, 0));
	MovablePoint waypoint(EAST, std::pair<int, int>(10, 1));
	for (auto &move : input) {
		moveShipOrWaypoint(ship, waypoint, move);
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
	std::cout << part2(parsedInput) << std::endl;
}

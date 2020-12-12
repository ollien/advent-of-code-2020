#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <numeric>
#include <optional>
#include <vector>

constexpr int NUM_DIRECTIONS = 4;
enum CardinalDirection { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

/**
 * MovablePoint represents either a ship or a waypoint
 */
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

	/**
	 * Move this point in the direction it is currently pointing, by the given magnitude
	 * @param value The magnitude to move by
	 */
	void move(int value) {
		this->move(value, this->direction);
	}

	/**
	 * Move this point in the direction given, by the given magnitude
	 * @param value The magnitude to move by
	 * @param direction The direction to move by
	 */
	void move(int value, CardinalDirection direction) {
		int delta = (direction == SOUTH || direction == WEST) ? -value : value;
		if (direction == NORTH || direction == SOUTH) {
			this->position.second += delta;
		} else {
			this->position.first += delta;
		}
	}

	/**
	 * Change the direction of this point by rotating left
	 * @param deg The degrees to ratate by - must be in increments of 90 deg
	 */
	void turnLeft(int deg) {
		// Three rights make a left - easier than dealing with negative mods
		this->addToDirection(3 * deg / 90);
	}

	/**
	 * Change the direction of this point by rotating right
	 * @param deg The degrees to ratate by - must be in increments of 90 deg
	 */
	void turnRight(int deg) {
		this->addToDirection(deg / 90);
	}

	/**
	 * Rotater this point about the origin
	 * @param degrees The number of degrees to move - must be in 90 degree increments
	 */
	void rotatePosition(int degrees) {
		int x = this->position.first;
		int y = this->position.second;
		// This is an awful way of writing a rotation, but it basically just hacking in the two parts of the rotation
		// matrix I need
		int numSteps = abs(degrees / 90);
		int stepAmount = degrees / numSteps;
		if (abs(stepAmount) != 90) {
			throw std::invalid_argument("Invalid rotation amount");
		}
		for (int i = 0; i < numSteps; i++) {
			std::swap(x, y);
			if (stepAmount < 0) {
				y *= -1;
			} else {
				x *= -1;
			}
		}

		this->position.first = x;
		this->position.second = y;
	}

 private:
	/**
	 * Add to the direction enum by the given amount, wrapping around in the positive direction only. Negative inputs
	 * will produce bad enumv alues
	 * @param delta The delta to rotate the direction by
	 */
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

/**
 * Parse the input to a usable format
 * @param input The puzzle input
 * @return std::vector<std::pair<char, int>> A vector of pairs of <directive, magnitude>
 */
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

/**
 * Move the ship (for part 1)
 * @param ship The ship to move
 * @param move The move to perform
 */
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

/**
 * Move the ship or the waypoint, depending on the direction (for part 2)
 * @param ship The ship to move
 * @param move The move to perform
 */
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

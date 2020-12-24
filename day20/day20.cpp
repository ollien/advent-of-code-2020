#include <folly/String.h>

#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <string>
#include <utility>
#include <vector>

constexpr auto TILE_ID_PATTERN = R"(Tile (\d+):)";
constexpr int NUM_CAMERA_LINES = 10;
// Plus one for the ID line, plus one for the newline after
constexpr int NUM_INPUT_BLOCK_LINES = NUM_CAMERA_LINES + 2;
constexpr char MONSTER_SIGNAL_CHAR = '#';
constexpr auto MONSTER_STR = 1 + R"(
                  # 
#    ##    ##    ###
 #  #  #  #  #  #   )";

/**
 * Represents a single frame captured by the camera
 */
class CameraFrame {
 public:
	CameraFrame(int id, const std::vector<std::string> &frame) : id(id), frame(frame) {
	}

	int getID() const {
		return this->id;
	}

	/**
	 * Get the contents of this frame
	 * @return const std::vector<std::string>& The frame contents
	 */
	const std::vector<std::string> &getFrame() const {
		return this->frame;
	}

	/**
	 * @return const std::string The top edge of this frame
	 */
	const std::string getTopEdge() const {
		return this->frame.front();
	}

	/**
	 * @return const std::string The bottom edge of this frame
	 */
	const std::string getBottomEdge() const {
		return this->frame.back();
	}

	/**
	 * @return const std::string The left edge of this frame
	 */
	const std::string getLeftEdge() const {
		std::string edge;
		std::transform(
			this->frame.cbegin(),
			this->frame.cend(),
			std::inserter(edge, edge.end()),
			[](const std::string &frameLine) { return frameLine.front(); });

		return edge;
	}

	/**
	 * @return const std::string The right edge of this frame
	 */
	const std::string getRightEdge() const {
		std::string edge;
		std::transform(
			this->frame.cbegin(),
			this->frame.cend(),
			std::inserter(edge, edge.end()),
			[](const std::string &frameLine) { return frameLine.back(); });

		return edge;
	}

	bool operator==(const CameraFrame &frame) const {
		return this->id == frame.id;
	}

	bool operator!=(const CameraFrame &frame) const {
		return !(*this == frame);
	}

	/**
	 * Flip this frame along the horizontal axis
	 */
	void flipFrameVertically() {
		std::reverse(this->frame.begin(), this->frame.end());
	}

	/**
	 * Flip this frame along the vertical axis
	 */
	void flipFrameHorizontally() {
		std::for_each(this->frame.begin(), this->frame.end(), [](std::string &frameLine) {
			std::reverse(frameLine.begin(), frameLine.end());
		});
	}

	/**
	 * Rotate this frame 90 degrees
	 */
	void rotateFrame90Deg() {
		// This algorithm works by rotating each "ring" of the frame 90 degrees
		// Consider the following 5x5 frame
		// a b c d e
		// f g h i j
		// k l m n o
		// p q r s t
		// u v w x y
		// First we rotate the ring that starts with "a b c d e", then "g h i", and once we hit "m", there's nothing to
		// rotate, so we're done.

		// Init our rotated frame to be the size of our existing (square) frame.
		std::vector<std::string> rotatedFrame(this->frame.size(), std::string(this->frame.at(0).size(), ' '));
		int ringStartIndex = 0;
		int ringEndIndex = this->frame.size() - 1;
		while (ringStartIndex < ringEndIndex) {
			// Top Edge -> Right Edge
			for (int i = ringStartIndex; i <= ringEndIndex; i++) {
				rotatedFrame.at(i).at(ringEndIndex) = frame.at(ringStartIndex).at(i);
			}

			// Right Edge -> Bottom Edge
			for (int i = ringStartIndex; i <= ringEndIndex; i++) {
				rotatedFrame.at(ringEndIndex).at(ringEndIndex - i + ringStartIndex) = frame.at(i).at(ringEndIndex);
			}

			// Bottom Edge -> Left Edge
			for (int i = ringStartIndex; i <= ringEndIndex; i++) {
				rotatedFrame.at(i).at(ringStartIndex) = frame.at(ringEndIndex).at(i);
			}

			// Left Edge -> Top Edge
			for (int i = ringStartIndex; i <= ringEndIndex; i++) {
				rotatedFrame.at(ringStartIndex).at(ringEndIndex - i + ringStartIndex) = frame.at(i).at(ringStartIndex);
			}

			ringStartIndex++;
			ringEndIndex--;
		}

		// Copy over the middle
		// Without this if, this overwrites legit rotated tiles when filling in the middle, if we have a board of an
		// even number size
		if (this->frame.size() % 2 != 0) {
			auto middle = this->frame.size() / 2;
			rotatedFrame.at(middle).at(middle) = frame.at(middle).at(middle);
		}

		this->frame = std::move(rotatedFrame);
	}

	/**
	 * Rotate this frame 180 degrees
	 */
	void rotateFrame180Deg() {
		this->rotateFrame90Deg();
		this->rotateFrame90Deg();
	}

	/**
	 * Rotate this frame 270 degrees
	 */
	void rotateFrame270Deg() {
		this->rotateFrame180Deg();
		this->rotateFrame90Deg();
	}

	/**
	 * Remove the border of this frame
	 */
	void removeFrameBorder() {
		std::vector<std::string> outFrame;
		std::transform(
			this->frame.cbegin() + 1,
			this->frame.cend() - 1,
			std::back_inserter(outFrame),
			[](const std::string &frameRow) { return frameRow.substr(1, frameRow.size() - 2); });

		this->frame = outFrame;
	}

 private:
	int id;
	std::vector<std::string> frame;
};

/**
 * Generates all of the possible transforms for a given camera frame.
 */
class TransformGenerator {
 public:
	class const_iterator {
	 public:
		using difference_type = int;
		using value_type = std::function<CameraFrame()>;
		using pointer = const value_type *;
		using reference = const value_type &;
		using iterator_category = std::input_iterator_tag;

		const_iterator &operator++() {
			++this->baseIterator;
			return *this;
		}

		const_iterator operator++(int) {
			const_iterator res = *this;
			++(*this);
			return res;
		}

		bool operator==(const const_iterator &other) {
			return this->baseIterator == other.baseIterator;
		}

		bool operator!=(const const_iterator &other) {
			return !(*this == other);
		}

		const value_type operator*() {
			return *(this->baseIterator);
		}

	 private:
		friend TransformGenerator;
		const_iterator(const TransformGenerator &container)
			: container(container), baseIterator(container.operationList.cbegin()) {
		}
		const_iterator(const TransformGenerator &container, const std::vector<value_type>::const_iterator &baseIterator)
			: container(container), baseIterator(baseIterator) {
		}

		const TransformGenerator &container;
		std::vector<value_type>::const_iterator baseIterator;
	};

	TransformGenerator(const CameraFrame &frame)
		: frame(frame),
		  operationList{
			  [frame]() { return frame; },
			  [frame = frame]() mutable {
				  frame.rotateFrame90Deg();
				  return frame;
			  },
			  [frame = frame]() mutable {
				  frame.rotateFrame180Deg();
				  return frame;
			  },
			  [frame = frame]() mutable {
				  frame.rotateFrame270Deg();
				  return frame;
			  },
			  [frame = frame]() mutable {
				  frame.flipFrameVertically();
				  return frame;
			  },
			  [frame = frame]() mutable {
				  frame.flipFrameHorizontally();
				  return frame;
			  },
			  [frame = frame]() mutable {
				  frame.flipFrameHorizontally();
				  frame.rotateFrame90Deg();
				  return frame;
			  },
			  // We don't need one for rotating 180 after flipping, as it's equivalent to flipping vertically
			  [frame = frame]() mutable {
				  frame.flipFrameHorizontally();
				  frame.rotateFrame270Deg();
				  return frame;
			  },
		  } {
	}

	const_iterator cbegin() const {
		return const_iterator(*this);
	}

	const_iterator cend() const {
		return const_iterator(*this, this->operationList.cend());
	}

 private:
	CameraFrame frame;
	std::vector<std::function<CameraFrame()>> operationList;
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
 * Get the frame ID from an input line containing one
 * @param line The frame ID line
 * @return int The frame ID
 */
int getFrameIDFromIDLine(const std::string &line) {
	std::regex pattern(TILE_ID_PATTERN);
	std::smatch matches;
	if (!std::regex_match(line, matches, pattern)) {
		throw std::invalid_argument("Invalid ID line");
	}

	return std::stoi(matches[1]);
}

/**
 * Parse the puzzle input
 * @param input The puzzle input
 * @return std::vector<CameraFrame> The frames from the camera input
 */
std::vector<CameraFrame> parseInput(const std::vector<std::string> &input) {
	int i = 0;
	std::vector<CameraFrame> cameraFrames;
	int currentFrameID;
	std::vector<std::string> currentCameraFrame;
	for (auto it = input.cbegin(); it != input.cend(); (++it, i++)) {
		const std::string &line = *it;
		int lineIndex = i % NUM_INPUT_BLOCK_LINES;
		if (lineIndex == 0) {
			currentFrameID = getFrameIDFromIDLine(line);
		} else if (line.size() > 0) {
			// If the line isn't blank, this is a frame of an image
			currentCameraFrame.push_back(line);
		} else {
			cameraFrames.emplace_back(currentFrameID, currentCameraFrame);
			currentCameraFrame.clear();
		}
	}

	// Handle the last frame
	cameraFrames.emplace_back(currentFrameID, currentCameraFrame);

	return cameraFrames;
}

std::ostream &operator<<(std::ostream &os, const CameraFrame &frame) {
	os << "Tile " << frame.getID() << ":" << std::endl;
	std::for_each(frame.getFrame().cbegin(), frame.getFrame().cend(), [&os](const std::string &frameLine) {
		os << frameLine << std::endl;
	});

	return os;
}

/**
 * Debugging method to print the entire board
 * @param board The board
 * @param maxRow The maximum row in the board
 * @param maxCol The maximum column in the board
 */
void printBoard(const std::map<std::pair<int, int>, CameraFrame> &board, int maxRow, int maxCol, int offset = 0) {
	CameraFrame emptyFrame(0, std::vector<std::string>(NUM_CAMERA_LINES, std::string(NUM_CAMERA_LINES, ' ')));
	for (int i = 0; i < maxRow; i++) {
		std::vector<CameraFrame> rowFrames;
		rowFrames.reserve(maxCol);
		for (int frameRow = 0; frameRow < NUM_CAMERA_LINES + offset; frameRow++) {
			for (int j = 0; j < maxCol; j++) {
				auto frame = board.find(std::make_pair(i, j));
				auto frameRowStr = (frame == board.end() ? emptyFrame : frame->second).getFrame().at(frameRow);
				std::cout << frameRowStr << " ";
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
}

/**
 * Check if the board has all tiles filled
 * @param board The board
 * @param maxRow The maximum row of the board
 * @param maxCol The maximum column of the board
 * @return true If the board is filled
 * @return false If the board is not filled
 */
bool isBoardFilled(const std::map<std::pair<int, int>, CameraFrame> board, int maxRow, int maxCol) {
	for (int i = 0; i < maxRow; i++) {
		for (int j = 0; j < maxCol; j++) {
			if (board.find(std::make_pair(i, j)) == board.end()) {
				return false;
			}
		}
	}

	return true;
}

/**
 * Find a possible matching frame for this frame
 * @param frameToMatch The frame to check
 * @param frameMatches A function to check if a given frame matches this one
 * @return std::optional<CameraFrame> The matching frame, if ti exists
 */
std::optional<CameraFrame> findPossibleFrame(
	const CameraFrame &frameToMatch, std::function<bool(const CameraFrame &)> frameMatches) {
	auto transformations = TransformGenerator(frameToMatch);
	auto result = std::find_if(transformations.cbegin(), transformations.cend(), [frameMatches](auto transformation) {
		CameraFrame transformedFrame = transformation();
		return frameMatches(transformedFrame);
	});
	if (result == transformations.cend()) {
		return std::nullopt;
	}

	return (*result)();
}

/**
 * Find a board that correctly lines everything up
 * @param startingFrame The frame to start with
 * @param frames All the other frames available for use
 * @param maxRow The maximum row size
 * @param maxCol The maximum column size
 * @return std::optional<std::map<std::pair<int, int>, CameraFrame>> The board that solves part 1
 */
std::optional<std::map<std::pair<int, int>, CameraFrame>> findLinedUpArrangement(
	const CameraFrame &startingFrame, const std::vector<CameraFrame> &frames, int maxRow, int maxCol) {
	std::map<std::pair<int, int>, CameraFrame> board;
	std::vector<CameraFrame> availableFrames(frames);
	board.emplace(std::make_pair(0, 0), startingFrame);
	for (int row = 0; row < maxRow; row++) {
		for (int col = 0; col < maxCol; col++) {
			if (row == 0 && col == 0) {
				continue;
			}

			bool found = false;
			for (const CameraFrame &currentFrame : availableFrames) {
				std::optional<CameraFrame> matchingFrame;
				if (col == 0) {
					CameraFrame &aboveFrame = board.at(std::make_pair(row - 1, col));
					matchingFrame = findPossibleFrame(currentFrame, [aboveFrame](const CameraFrame &frame) {
						return aboveFrame.getBottomEdge() == frame.getTopEdge();
					});
				} else {
					CameraFrame &leftFrame = board.at(std::make_pair(row, col - 1));
					matchingFrame = findPossibleFrame(currentFrame, [leftFrame](const CameraFrame &frame) {
						return leftFrame.getRightEdge() == frame.getLeftEdge();
					});
				}

				if (matchingFrame) {
					found = true;
					board.emplace(std::make_pair(row, col), *matchingFrame);
					availableFrames.erase(std::remove(availableFrames.begin(), availableFrames.end(), currentFrame));
					break;
				}
			}
			if (!found) {
				return std::nullopt;
			}
		}
	}

	if (!isBoardFilled(board, maxRow, maxCol)) {
		return std::nullopt;
	}

	return board;
}

/**
 * Determine the size of the board given the captured frames
 * @param frames The frames to use on the board
 * @return int The board size
 */
int calculateBoardSize(const std::vector<CameraFrame> &frames) {
	return sqrt(frames.size());
}

/**
 * Find a board that correctly lines everything up
 * @param frames All of the frames
 * @return std::optional<std::map<std::pair<int, int>, CameraFrame>> The board that solves part 1
 */
std::map<std::pair<int, int>, CameraFrame> findLinedUpArrangement(const std::vector<CameraFrame> &frames) {
	int boardSize = calculateBoardSize(frames);
	for (const CameraFrame &frame : frames) {
		std::vector<CameraFrame> availableFrames(frames);
		availableFrames.erase(std::remove(availableFrames.begin(), availableFrames.end(), frame));

		const TransformGenerator transforms(frame);
		for (auto it = transforms.cbegin(); it != transforms.cend(); ++it) {
			auto transformation = *it;
			CameraFrame transformedFrame = transformation();
			auto res = findLinedUpArrangement(transformedFrame, availableFrames, boardSize, boardSize);
			if (res) {
				return *res;
			}
		}
	}

	throw std::invalid_argument("No solution for input");
}

/**
 * Get the size of a monster, given a strong representing one.
 * @param monster The monster string
 * @return std::pair<int, int> The height and width of the monster, as a pair.
 */
std::pair<int, int> getMonsterDimensions(const std::string &monster) {
	int height = std::count(monster.cbegin(), monster.cend(), '\n') + 1;
	std::vector<std::string> lines;
	folly::split("\n", monster, lines);
	auto longestIt =
		std::max_element(lines.cbegin(), lines.cend(), [](const std::string &line1, const std::string &line2) {
			return line1.size() < line2.size();
		});

	int width = longestIt->size();

	return std::make_pair(height, width);
}

/**
 * Join a board's frames together into one big board.
 * @param board The board to join.
 * @param maxRow The highest row in the board
 * @param maxCol The highest col in the board
 * @return std::vector<std::string> The joined board
 */
std::vector<std::string> joinFullBoard(
	const std::map<std::pair<int, int>, CameraFrame> &board, int maxRow, int maxCol) {
	std::vector<std::string> outBoard;
	// Get an arbitrary frame - doesn't matter what it is, we just need a size.
	auto firstFrame = *board.begin();
	for (int i = 0; i < maxRow; i++) {
		for (int frameRow = 0; frameRow < firstFrame.second.getFrame().size(); frameRow++) {
			outBoard.push_back("");
			std::string &outRow = outBoard.back();
			for (int j = 0; j < maxCol; j++) {
				const CameraFrame &frame = board.at(std::make_pair(i, j));
				std::string frameRowItems = frame.getFrame().at(frameRow);
				outRow += frameRowItems;
			}
		}
	}

	return outBoard;
}

/**
 * Get the number of characters that contain a monster, within the monster dimensions, starting at (row, col). This only scans for a single monster.
 * @param frame The frame to scan
 * @param row The row to start at
 * @param col The col to start at
 * @return int The number of monster characters in this area.
 */
int getNumMonsterChars(const CameraFrame &frame, int row, int col) {
	std::pair<int, int> monsterDimensions = getMonsterDimensions(MONSTER_STR);
	std::vector<std::string> monsterLines;
	folly::split("\n", MONSTER_STR, monsterLines);

	int total = 0;
	for (int i = 0; i < monsterDimensions.first; i++) {
		bool foundMonster = true;
		for (int j = 0; j < monsterDimensions.second; j++) {
			char monsterChar = monsterLines.at(i).at(j);
			char frameChar = frame.getFrame().at(row + i).at(col + j);
			if (monsterChar == MONSTER_SIGNAL_CHAR && frameChar != MONSTER_SIGNAL_CHAR) {
				foundMonster = false;
				break;
			}

			total += (monsterChar == MONSTER_SIGNAL_CHAR);
		}
		// If this line didn't find a monster, there can't be any true monster chars
		if (!foundMonster) {
			return 0;
		}
	}

	return total;
}

long part1(const std::map<std::pair<int, int>, CameraFrame> &board, int boardSize) {
	return 1L * board.at(std::make_pair(0, 0)).getID() * board.at(std::make_pair(0, boardSize - 1)).getID() *
		   board.at(std::make_pair(boardSize - 1, 0)).getID() *
		   board.at(std::make_pair(boardSize - 1, boardSize - 1)).getID();
}


long part2(const std::map<std::pair<int, int>, CameraFrame> &rawBoard, int boardSize) {
	// Remove all of the borders from the board
	std::map<std::pair<int, int>, CameraFrame> board(rawBoard);
	std::for_each(board.begin(), board.end(), [](std::pair<const std::pair<int, int>, CameraFrame> &frame) {
		frame.second.removeFrameBorder();
	});

	std::pair<int, int> monsterDimensions = getMonsterDimensions(MONSTER_STR);
	auto fullBoard = joinFullBoard(board, boardSize, boardSize);
	auto fullBoardFrame = CameraFrame(0, fullBoard);
	int totalSignalChars =
		std::accumulate(fullBoard.cbegin(), fullBoard.cend(), 0, [](int total, const std::string &boardRow) {
			return total + std::count(boardRow.cbegin(), boardRow.cend(), MONSTER_SIGNAL_CHAR);
		});

	auto transforms = TransformGenerator(fullBoardFrame);
	for (auto it = transforms.cbegin(); it != transforms.cend(); it++) {
		auto transformedFrame = (*it)();
		int totalMonsterChars = 0;
		for (int i = 0; i < fullBoard.size() - monsterDimensions.first; i++) {
			for (int j = 0; j < fullBoard.at(0).size() - monsterDimensions.second; j++) {
				int monsterChars = getNumMonsterChars(transformedFrame, i, j);
				totalMonsterChars += monsterChars;
			}
		}

		// Only one orientation will match a monster
		if (totalMonsterChars > 0) {
			return totalSignalChars - totalMonsterChars;
		}
	}

	throw std::invalid_argument("No valid solution");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto parsedInput = parseInput(input);
	int boardSize = calculateBoardSize(parsedInput);
	auto board = findLinedUpArrangement(parsedInput);

	std::cout << part1(board, boardSize) << std::endl;
	std::cout << part2(board, boardSize) << std::endl;
}

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
#include <vector>

constexpr auto TILE_ID_PATTERN = R"(Tile (\d+):)";
constexpr int NUM_CAMERA_LINES = 10;
// Plus one for the ID line, plus one for the newline after
constexpr int NUM_INPUT_BLOCK_LINES = NUM_CAMERA_LINES + 2;

class CameraFrame {
 public:
	CameraFrame(int id, const std::vector<std::string> &frame) : id(id), frame(frame) {
	}

	int getID() const {
		return this->id;
	}

	const std::vector<std::string> &getFrame() const {
		return this->frame;
	}

	const std::string getTopEdge() const {
		return this->frame.front();
	}

	const std::string getBottomEdge() const {
		return this->frame.back();
	}

	const std::string getLeftEdge() const {
		std::string edge;
		std::transform(
			this->frame.cbegin(),
			this->frame.cend(),
			std::inserter(edge, edge.end()),
			[](const std::string &frameLine) { return frameLine.front(); });

		return edge;
	}

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

	void flipFrameVertically() {
		std::reverse(this->frame.begin(), this->frame.end());
	}

	void flipFrameHorizontally() {
		std::for_each(this->frame.begin(), this->frame.end(), [](std::string &frameLine) {
			std::reverse(frameLine.begin(), frameLine.end());
		});
	}

	void rotateFrame90Deg() {
		// This algorithm works by rotating each "ring" of the frame 90 degrees
		// Consider the following 5x5 frame
		// a b c d e
		// f g h i j
		// k l m n o
		// p q r s t
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

		// Fill in the middle
		auto middle = this->frame.size() / 2;
		rotatedFrame.at(middle).at(middle) = frame.at(middle).at(middle);

		this->frame = std::move(rotatedFrame);
	}

	void rotateFrame180Deg() {
		this->rotateFrame90Deg();
		this->rotateFrame90Deg();
	}

	void rotateFrame270Deg() {
		this->rotateFrame180Deg();
		this->rotateFrame90Deg();
	}

 private:
	int id;
	std::vector<std::string> frame;
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

int getFrameIDFromIDLine(const std::string &line) {
	std::regex pattern(TILE_ID_PATTERN);
	std::smatch matches;
	if (!std::regex_match(line, matches, pattern)) {
		throw std::invalid_argument("Invalid ID line");
	}

	return std::stoi(matches[1]);
}

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

void printBoard(const std::map<std::pair<int, int>, CameraFrame> &board, int maxRow, int maxCol) {
	CameraFrame emptyFrame(0, std::vector<std::string>(NUM_CAMERA_LINES, std::string(NUM_CAMERA_LINES, ' ')));
	for (int i = 0; i < maxRow; i++) {
		std::vector<CameraFrame> rowFrames;
		rowFrames.reserve(maxCol);
		for (int frameRow = 0; frameRow < NUM_CAMERA_LINES; frameRow++) {
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

std::vector<CameraFrame> getPossibleTransforms(const CameraFrame &frame) {
	CameraFrame identity(frame);
	CameraFrame rotated90Deg(frame);
	rotated90Deg.rotateFrame90Deg();
	CameraFrame rotated180Deg(frame);
	rotated180Deg.rotateFrame180Deg();
	CameraFrame rotated270Deg(frame);
	rotated270Deg.rotateFrame270Deg();
	CameraFrame flippedVertically(frame);
	flippedVertically.flipFrameVertically();
	CameraFrame flippedHorizontally(frame);
	flippedHorizontally.flipFrameHorizontally();
	CameraFrame flippedAndRotated90(flippedHorizontally);
	flippedAndRotated90.rotateFrame90Deg();
	// We don't need flip and rotate 180 because it is equivalent to flipVertically
	CameraFrame flippedAndRotated270(flippedHorizontally);
	flippedAndRotated270.rotateFrame270Deg();
	std::vector<CameraFrame> operations{
		identity,
		rotated90Deg,
		rotated180Deg,
		rotated270Deg,
		flippedVertically,
		flippedHorizontally,
		flippedAndRotated90,
		flippedAndRotated270};

	return operations;
}

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

std::optional<CameraFrame> findPossibleFrame(
	const std::vector<CameraFrame> &availableFrames, std::function<bool(const CameraFrame &)> findFrame) {
	auto result = std::find_if(availableFrames.cbegin(), availableFrames.cend(), findFrame);
	if (result == availableFrames.cend()) {
		return std::nullopt;
	}

	return *result;
}

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
				auto transformed = getPossibleTransforms(currentFrame);
				for (const CameraFrame &currentTransformedFrame : transformed) {
					std::optional<CameraFrame> matchingFrame;
					if (col == 0) {
						CameraFrame &aboveFrame = board.at(std::make_pair(row - 1, col));
						matchingFrame = findPossibleFrame(transformed, [aboveFrame](const CameraFrame &frame) {
							return aboveFrame.getBottomEdge() == frame.getTopEdge();
						});
					} else {
						CameraFrame &leftFrame = board.at(std::make_pair(row, col - 1));
						matchingFrame = findPossibleFrame(transformed, [leftFrame](const CameraFrame &frame) {
							return leftFrame.getRightEdge() == frame.getLeftEdge();
						});
					}

					if (matchingFrame) {
						found = true;
						board.emplace(std::make_pair(row, col), *matchingFrame);
						availableFrames.erase(
							std::remove(availableFrames.begin(), availableFrames.end(), currentFrame));
						// printBoard(board, maxRow, maxCol);
						break;
					}
				}
				if (found) {
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

long part1(const std::vector<CameraFrame> &frames) {
	int boardSize = sqrt(frames.size());
	for (const CameraFrame &frame : frames) {
		std::vector<CameraFrame> availableFrames(frames);
		availableFrames.erase(std::remove(availableFrames.begin(), availableFrames.end(), frame));

		auto transformed = getPossibleTransforms(frame);
		for (const CameraFrame &transformedFrame : transformed) {
			auto res = findLinedUpArrangement(transformedFrame, availableFrames, boardSize, boardSize);
			if (res) {
				return 1L * res->at(std::make_pair(0, 0)).getID() * res->at(std::make_pair(0, boardSize - 1)).getID() *
					   res->at(std::make_pair(boardSize - 1, 0)).getID() *
					   res->at(std::make_pair(boardSize - 1, boardSize - 1)).getID();
			}
		}
	}

	throw std::invalid_argument("No solution for input");
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

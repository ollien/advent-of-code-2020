#include <folly/String.h>

#include <algorithm>
#include <execution>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string_view>
#include <vector>

constexpr auto ACCUMULATOR_INSTRUCTION = "acc";
constexpr auto JUMP_INSTRUCTION = "jmp";
constexpr auto NOP_INSTRUCTION = "nop";

class ProgramLine {
 public:
	ProgramLine(std::string instruction, int value) : instruction(instruction), value(value) {
	}

	const std::string &getInstruction() const {
		return this->instruction;
	}

	int getValue() const {
		return this->value;
	}

 private:
	std::string instruction;
	int value;
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

std::vector<ProgramLine> parseProgramLines(const std::vector<std::string> &input) {
	std::vector<ProgramLine> lines;
	lines.reserve(input.size());
	std::transform(input.cbegin(), input.cend(), std::back_inserter(lines), [](const std::string &line) {
		std::vector<std::string> lineComponents;
		folly::split(" ", line, lineComponents);
		if (lineComponents.size() > 2) {
			throw new std::invalid_argument("Invalid programspec");
		}

		return ProgramLine(lineComponents[0], std::stoi(lineComponents[1]));
	});

	return lines;
}

int part1(const std::vector<ProgramLine> &lines) {
	std::set<int> visitedLines;
	int accumulator = 0;
	int programCounter = 0;
	while (true) {
		ProgramLine line = lines.at(programCounter);
		if (visitedLines.find(programCounter) != visitedLines.end()) {
			return accumulator;
		}
		visitedLines.insert(programCounter);

		if (line.getInstruction() == JUMP_INSTRUCTION) {
			programCounter += line.getValue();
			continue;
		} else if (line.getInstruction() == ACCUMULATOR_INSTRUCTION) {
			accumulator += line.getValue();
		} else if (line.getInstruction() != NOP_INSTRUCTION) {
			throw new std::invalid_argument("Invalid instruction");
		}

		programCounter++;
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	auto programLines = parseProgramLines(input);

	std::cout << part1(programLines) << std::endl;
}

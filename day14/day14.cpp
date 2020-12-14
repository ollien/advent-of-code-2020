#include <folly/String.h>

#include <algorithm>
#include <cassert>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <vector>

constexpr char IGNORE_CHAR = 'X';
constexpr auto MASK_PATTERN = R"(mask = ([X0-9]+))";
constexpr auto MEM_PATTERN = R"(mem\[(\d+)\] = (\d+))";

class InstructionBlock {
 public:
	InstructionBlock(std::string mask, std::vector<std::pair<int, int>> storeInstructions)
		: mask(mask), storeInstructions(storeInstructions) {
	}

	// We are dealing with a 36 bit numebr so we must use a long long to guarantee we can fit it
	long long maskNumber(long long num) const {
		long long res = num;
		for (int i = 0; i < this->mask.size(); i++) {
			char maskChar = this->mask.at(this->mask.size() - i - 1);
			if (maskChar == IGNORE_CHAR) {
				continue;
			} else if (maskChar == '0') {
				res &= ~(1LL << i);
			} else if (maskChar == '1') {
				res |= (1LL << i);
			}
		}

		return res;
	}

	const std::vector<std::pair<int, int>> &getStoreInstructions() const {
		return this->storeInstructions;
	}

 private:
	std::string mask;
	std::vector<std::pair<int, int>> storeInstructions;
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

std::vector<InstructionBlock> parseInput(const std::vector<std::string> &input) {
	std::vector<InstructionBlock> blocks;
	std::regex maskExpression(MASK_PATTERN);
	std::regex memExpression(MEM_PATTERN);
	std::string currentMask;
	std::vector<std::pair<int, int>> currentStoreInstructions;
	for (auto it = input.cbegin(); it != input.cend(); it++) {
		auto line = *it;
		std::smatch matches;
		if (std::regex_match(line, matches, maskExpression)) {
			// We don't want to emplace on the first mask we find
			if (it != input.cbegin()) {
				blocks.emplace_back(currentMask, currentStoreInstructions);
				currentStoreInstructions.clear();
			}

			currentMask = matches[1];
		} else if (std::regex_match(line, matches, memExpression)) {
			int address = std::stoi(matches[1]);
			int value = std::stoi(matches[2]);
			currentStoreInstructions.emplace_back(address, value);
		}
	}

	// Put in the last one we've found
	blocks.emplace_back(currentMask, currentStoreInstructions);

	return blocks;
}

long long part1(const std::vector<InstructionBlock> &instructions) {
	std::unordered_map<int, long long> memory;
	for (const InstructionBlock &instruction : instructions) {
		for (const std::pair<int, int> &storeInstruction : instruction.getStoreInstructions()) {
			auto maskedStorage = instruction.maskNumber(storeInstruction.second);
			memory[storeInstruction.first] = maskedStorage;
		}
	}

	return std::accumulate(
		memory.cbegin(), memory.cend(), 0LL, [](long long total, std::pair<int, long long> memoryItem) {
			return total + memoryItem.second;
		});
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

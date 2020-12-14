#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <vector>

constexpr char IGNORE_CHAR = 'X';
constexpr auto MASK_PATTERN = R"(mask = ([X0-9]+))";
constexpr auto MEM_PATTERN = R"(mem\[(\d+)\] = (\d+))";

/**
 * Represents a block of instructions with a given mask
 */
class InstructionBlock {
 public:
	InstructionBlock(std::string mask, std::vector<std::pair<int, int>> storeInstructions)
		: mask(mask), storeInstructions(storeInstructions) {
	}

	/**
	 * Mask a value to be stored in memory, in accordance with the mask for this block
	 * @param num The number to mask
	 * @return long long The masked value
	 */
	long long maskValue(long long num) const {
		// We are dealing with a 36 bit numebr so we must use a long long to guarantee we can fit it
		long long res = num;
		for (std::string::size_type i = 0; i < this->mask.size(); i++) {
			char maskChar = this->mask.at(this->mask.size() - i - 1);
			if (maskChar == IGNORE_CHAR) {
				continue;
			} else {
				res = this->setBitAt(res, i, maskChar);
			}
		}

		return res;
	}

	/**
	 * Mask a memory address, in accordance with the mask for this block
	 * @param address The address to mask
	 * @return std::vector<long long> All of the possible masked addresses
	 */
	std::vector<long long> maskMemoryAddress(long long address) const {
		return this->recursivelyMaskAddresses(address);
	}

	const std::vector<std::pair<int, int>> &getStoreInstructions() const {
		return this->storeInstructions;
	}

 private:
	std::string mask;
	std::vector<std::pair<int, int>> storeInstructions;

	/**
	 * Set a bit at the given position
	 * @param value The value set the value within
	 * @param position The position to set the bit at
	 * @param bit The bit to set, either 0 or 1
	 * @return long long The value with the bit set
	 */
	long long setBitAt(long long value, int position, char bit) const {
		auto res = value;
		if (bit == '0') {
			res &= ~(1LL << position);
		} else if (bit == '1') {
			res |= (1LL << position);
		} else {
			throw std::invalid_argument("bit must be zero or one");
		}

		return res;
	}

	/**
	 * Perform the masking needed for the memory address
	 * @param address The address to mask
	 * @param startIdx The index to start the search at
	 * @return std::vector<long long> All of the possible masked addresses
	 */
	std::vector<long long> recursivelyMaskAddresses(long long address, std::string::size_type startIdx = 0) const {
		std::vector<long long> results;
		long long res = address;
		for (std::string::size_type i = startIdx; i < this->mask.size(); i++) {
			char maskChar = this->mask.at(this->mask.size() - i - 1);
			if (maskChar == IGNORE_CHAR) {
				auto masked0 = this->setBitAt(res, i, '0');
				auto masked1 = this->setBitAt(res, i, '1');
				auto results0 = this->recursivelyMaskAddresses(masked0, i + 1);
				auto results1 = this->recursivelyMaskAddresses(masked1, i + 1);
				results.insert(results.end(), results0.begin(), results0.end());
				results.insert(results.end(), results1.begin(), results1.end());
			} else if (maskChar == '1') {
				res = this->setBitAt(res, i, maskChar);
			}
		}

		results.push_back(res);
		return results;
	}
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
 * Parse the input into InstructionBlocks
 * @param input The input to parse
 * @return std::vector<InstructionBlock> The parsed input
 */
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
			auto maskedValue = instruction.maskValue(storeInstruction.second);
			memory[storeInstruction.first] = maskedValue;
		}
	}

	return std::accumulate(
		memory.cbegin(), memory.cend(), 0LL, [](long long total, std::pair<int, long long> memoryItem) {
			return total + memoryItem.second;
		});
}

long long part2(const std::vector<InstructionBlock> &instructions) {
	std::unordered_map<long long, long long> memory;
	for (const InstructionBlock &instruction : instructions) {
		for (const std::pair<int, int> &storeInstruction : instruction.getStoreInstructions()) {
			for (const auto maskedAddress : instruction.maskMemoryAddress(storeInstruction.first)) {
				memory[maskedAddress] = storeInstruction.second;
			}
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
	std::cout << part2(parsedInput) << std::endl;
}

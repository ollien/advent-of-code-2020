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

auto constexpr LINE_PATTERN = R"((.*) bags? contain (.*)\.)";
auto constexpr BAG_PATTERN = R"((\d) (.*) bags?)";
auto constexpr NO_OTHER_BAGS = "no other bags";
auto constexpr BAG_DELIM = ", ";
auto constexpr DESIRED_BAG = "shiny gold";

class ContainedBag {
 public:
	ContainedBag(std::string color, int quantity) : color(color), quantity(quantity) {
	}

	const std::string &getColor() const {
		return this->color;
	}

	int getQuantity() const {
		return this->quantity;
	}

	bool operator==(ContainedBag bag) {
		return bag.getColor() == this->getColor() && bag.getQuantity() == this->getQuantity();
	}

 private:
	std::string color;
	int quantity;
};

// I would use a set for the value but I don't want to deal with hash functions right now
using BagMap = std::map<std::string, std::vector<ContainedBag>>;

std::vector<std::string> readInput(const std::string &filename) {
	std::vector<std::string> input;
	std::string line;
	std::ifstream file(filename);
	while (std::getline(file, line)) {
		input.push_back(line);
	}

	return input;
}

std::pair<std::string, std::vector<ContainedBag>> parseInputLine(const std::string &line) {
	std::regex lineExpression(LINE_PATTERN);
	std::smatch lineMatches;
	if (!std::regex_match(line, lineMatches, lineExpression)) {
		throw new std::invalid_argument("Invalid input line");
	}

	std::string bagName = lineMatches[1];
	std::string allUnparsedBags = lineMatches[2];
	std::vector<std::string> unparsedBags;
	folly::split(BAG_DELIM, allUnparsedBags, unparsedBags);
	std::vector<ContainedBag> containedBags;
	for (const std::string &unparsedBag : unparsedBags) {
		// Check for no bags contained
		if (unparsedBag == NO_OTHER_BAGS) {
			continue;
		}

		// Check for some quantity of bags contained
		std::regex bagExpression(BAG_PATTERN);
		std::smatch bagMatches;
		if (!std::regex_match(unparsedBag, bagMatches, bagExpression)) {
			throw new std::invalid_argument("Invalid bagspec");
		}

		ContainedBag containedBag(bagMatches[2], std::stoi(bagMatches[1]));
		containedBags.push_back(containedBag);
	}

	return std::pair<std::string, std::vector<ContainedBag>>(bagName, containedBags);
}

BagMap makeBagMap(const std::vector<std::string> &input) {
	std::map<std::string, std::vector<ContainedBag>> bags;
	std::transform(input.cbegin(), input.cend(), std::inserter(bags, bags.begin()), [](const std::string &line) {
		return parseInputLine(line);
	});

	return bags;
}

bool doesBagContain(BagMap bagMap, const std::string &originBag, const std::string &desiredBag) {
	std::stack<std::string> toVisit;
	std::set<std::string> visited;
	toVisit.emplace(originBag);
	while (!toVisit.empty()) {
		std::string visiting = toVisit.top();
		toVisit.pop();
		if (visited.find(visiting) != visited.end()) {
			continue;
		}

		if (visiting == desiredBag) {
			return true;
		}

		for (ContainedBag bag : bagMap.at(visiting)) {
			toVisit.emplace(std::string(bag.getColor()));
		}
	}

	return false;
}

int part1(const std::vector<std::string> &input) {
	auto bagMap = makeBagMap(input);
	int count = 0;
	for (auto bagEntry : bagMap) {
		const std::string &bagName = bagEntry.first;
		if (bagName == DESIRED_BAG) {
			continue;
		}

		count += doesBagContain(bagMap, bagName, DESIRED_BAG);
	}

	return count;
}

int part2(const std::vector<std::string> &input) {
	auto bagMap = makeBagMap(input);
	int count = 0;

	std::stack<std::string> toVisit;
	toVisit.emplace(DESIRED_BAG);
	while (!toVisit.empty()) {
		std::string visiting = toVisit.top();
		toVisit.pop();

		for (ContainedBag bag : bagMap.at(visiting)) {
			count += bag.getQuantity();
			// Enqueue this bagQuantity times. This won't account for cycles (I hope there aren't any), and is slow, but
			// will work.
			for (int i = 0; i < bag.getQuantity(); i++) {
				toVisit.emplace(std::string(bag.getColor()));
			}
		}
	}

	return count;
}

int recursivePart2(const BagMap &bagMap, const std::string &toVisit = DESIRED_BAG) {
	std::vector<ContainedBag> childBags = bagMap.at(toVisit);

	return std::accumulate(childBags.cbegin(), childBags.cend(), 0, [&](int total, const ContainedBag &bag) {
		return total + bag.getQuantity() + bag.getQuantity() * recursivePart2(bagMap, bag.getColor());
	});
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	std::cout << part1(input) << std::endl;
	std::cout << part2(input) << std::endl;
	std::cout << recursivePart2(makeBagMap(input)) << std::endl;
}

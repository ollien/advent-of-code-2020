#include <folly/String.h>

#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

constexpr auto NEARBY_TICKETS_HEADER = "nearby tickets:";
constexpr auto YOUR_TICKET_HEADER = "your ticket:";
constexpr auto RANGE_PATTERN = R"((\d+)-(\d+) or (\d+)-(\d+))";

class TicketSpec {
 public:
	TicketSpec(
		std::vector<int> &ourTicket, std::vector<std::vector<int>> &otherTickets,
		std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> &fieldRanges)
		: ourTicket(ourTicket), otherTickets(otherTickets), fieldRanges(fieldRanges) {
	}

	TicketSpec(
		std::vector<int> &&ourTicket, std::vector<std::vector<int>> &&otherTickets,
		std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> &&fieldRanges)
		: ourTicket(std::move(ourTicket)), otherTickets(std::move(otherTickets)), fieldRanges(std::move(fieldRanges)) {
	}

	const std::vector<int> &getOurTicket() const {
		return this->ourTicket;
	}

	const std::vector<std::vector<int>> &getOtherTickets() const {
		return this->otherTickets;
	}

	const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> &getFieldRanges() const {
		return this->fieldRanges;
	}

 private:
	std::vector<int> ourTicket;
	std::vector<std::vector<int>> otherTickets;
	// This has to be the grossest type signature I've written in a while, but it's of
	// a-b or c-d maps to pair<a, b> or pair<c ,d>
	std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> fieldRanges;
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

template <typename Iter>
std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> findPairs(Iter start, Iter end) {
	std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> pairs;
	std::regex rangeExpression(RANGE_PATTERN);
	for (auto textIter = start; textIter != end; ++textIter) {
		std::smatch matches;
		if (!std::regex_search(*textIter, matches, rangeExpression)) {
			throw new std::invalid_argument("Invalid input");
		}
		std::pair<int, int> range1(std::stoi(matches[1]), std::stoi(matches[2]));
		std::pair<int, int> range2(std::stoi(matches[3]), std::stoi(matches[4]));
		pairs.emplace_back(std::move(range1), std::move(range2));
	}

	return pairs;
}

std::vector<int> parseTicket(const std::string &rawTicket) {
	std::vector<int> ticket;
	folly::split(",", rawTicket, ticket);

	return ticket;
}

template <typename Iter>
std::vector<std::vector<int>> parseNearbyTickets(Iter start, Iter end) {
	std::vector<std::vector<int>> nearbyTickets;
	for (auto it = start; it != end; ++it) {
		auto ticket = parseTicket(*it);
		nearbyTickets.push_back(std::move(ticket));
	}

	return nearbyTickets;
}

TicketSpec parseInput(const std::vector<std::string> &input) {
	auto ruleEnd = std::find(input.cbegin(), input.cend(), "");
	auto yourTicketBegin = std::find(input.cbegin(), input.cend(), YOUR_TICKET_HEADER);
	auto nearbyTicketsBegin = std::find(input.cbegin(), input.cend(), NEARBY_TICKETS_HEADER);
	auto pairs = findPairs(input.begin(), ruleEnd);
	auto nearbyTickets = parseNearbyTickets(nearbyTicketsBegin + 1, input.cend());
	auto yourTicket = parseTicket(*(yourTicketBegin + 1));

	return TicketSpec(std::move(yourTicket), std::move(nearbyTickets), std::move(pairs));
}

bool inRangesForTicket(int value, const TicketSpec &ticketSpec) {
	return std::any_of(
		ticketSpec.getFieldRanges().cbegin(),
		ticketSpec.getFieldRanges().cend(),
		[value](const std::pair<std::pair<int, int>, std::pair<int, int>> &ranges) {
			auto range1 = ranges.first;
			auto range2 = ranges.second;
			return (value >= range1.first && value <= range1.second) ||
				   (value >= range2.first && value <= range2.second);
		});
}

int part1(const TicketSpec &ticketSpec) {
	return std::accumulate(
		ticketSpec.getOtherTickets().cbegin(),
		ticketSpec.getOtherTickets().cend(),
		0,
		[&ticketSpec](int total, const std::vector<int> &ticket) {
			return total + std::accumulate(ticket.cbegin(), ticket.cend(), 0, [&ticketSpec](int subtotal, int field) {
					   return inRangesForTicket(field, ticketSpec) ? subtotal : subtotal + field;
				   });
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

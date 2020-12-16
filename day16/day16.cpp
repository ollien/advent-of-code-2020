#include <folly/String.h>

#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <regex>
#include <set>
#include <string>
#include <vector>

constexpr auto NEARBY_TICKETS_HEADER = "nearby tickets:";
constexpr auto YOUR_TICKET_HEADER = "your ticket:";
constexpr auto FIELD_PATTERN = R"((.*): (\d+)-(\d+) or (\d+)-(\d+))";

// This has to be the grossest type signature I've written in a while, but it's of
// a-b or c-d maps to pair<a, b> or pair<c ,d>
using RangeSpec = std::pair<std::pair<int, int>, std::pair<int, int>>;

class TicketField {
 public:
	TicketField(std::string &name, RangeSpec &ranges) : name(name), ranges(ranges) {
	}
	TicketField(std::string &&name, RangeSpec &&ranges) : name(std::move(name)), ranges(std::move(ranges)) {
	}

	bool operator<(const TicketField other) const {
		return this->getName() < other.getName();
	}

	bool isValueInRanges(int value) const {
		auto range1 = ranges.first;
		auto range2 = ranges.second;
		return (value >= range1.first && value <= range1.second) || (value >= range2.first && value <= range2.second);
	}

	const std::string &getName() const {
		return this->name;
	}

	const RangeSpec &getRanges() const {
		return this->ranges;
	}

 private:
	std::string name;
	RangeSpec ranges;
};

class TicketSpec {
 public:
	TicketSpec(
		std::vector<int> &ourTicket, std::vector<std::vector<int>> &otherTickets, std::vector<TicketField> &fields)
		: ourTicket(ourTicket), otherTickets(otherTickets), fields(fields) {
	}

	TicketSpec(
		std::vector<int> &&ourTicket, std::vector<std::vector<int>> &&otherTickets, std::vector<TicketField> &&fields)
		: ourTicket(std::move(ourTicket)), otherTickets(std::move(otherTickets)), fields(std::move(fields)) {
	}

	const std::vector<int> &getOurTicket() const {
		return this->ourTicket;
	}

	const std::vector<std::vector<int>> &getOtherTickets() const {
		return this->otherTickets;
	}

	const std::vector<TicketField> &getFields() const {
		return this->fields;
	}

 private:
	std::vector<int> ourTicket;
	std::vector<std::vector<int>> otherTickets;
	std::vector<TicketField> fields;
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
std::vector<TicketField> parseFields(Iter start, Iter end) {
	std::vector<TicketField> fields;
	std::regex rangeExpression(FIELD_PATTERN);
	for (auto textIter = start; textIter != end; ++textIter) {
		std::smatch matches;
		if (!std::regex_search(*textIter, matches, rangeExpression)) {
			throw new std::invalid_argument("Invalid input");
		}
		std::pair<int, int> range1(std::stoi(matches[2]), std::stoi(matches[3]));
		std::pair<int, int> range2(std::stoi(matches[4]), std::stoi(matches[5]));
		RangeSpec ticketRange(std::move(range1), std::move(range2));
		fields.emplace_back(matches[1].str(), std::move(ticketRange));
	}

	return fields;
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
	auto fieldsEnd = std::find(input.cbegin(), input.cend(), "");
	auto yourTicketBegin = std::find(input.cbegin(), input.cend(), YOUR_TICKET_HEADER);
	auto nearbyTicketsBegin = std::find(input.cbegin(), input.cend(), NEARBY_TICKETS_HEADER);
	auto pairs = parseFields(input.begin(), fieldsEnd);
	auto nearbyTickets = parseNearbyTickets(nearbyTicketsBegin + 1, input.cend());
	auto yourTicket = parseTicket(*(yourTicketBegin + 1));

	return TicketSpec(std::move(yourTicket), std::move(nearbyTickets), std::move(pairs));
}
bool inRangesForTicket(int value, const TicketSpec &ticketSpec) {
	return std::any_of(
		ticketSpec.getFields().cbegin(), ticketSpec.getFields().cend(), [value](const TicketField &field) {
			return field.isValueInRanges(value);
		});
}

std::vector<std::vector<int>> getValidTickets(const TicketSpec &ticketSpec) {
	// There might be a nicer way than copying these but frankly I can't think of one that's clean.
	std::vector<std::vector<int>> allTickets(ticketSpec.getOtherTickets());
	allTickets.push_back(ticketSpec.getOurTicket());
	allTickets.erase(
		std::remove_if(
			allTickets.begin(),
			allTickets.end(),
			[&ticketSpec](const std::vector<int> &ticket) {
				return !std::all_of(ticket.cbegin(), ticket.cend(), [&ticketSpec](int field) {
					return inRangesForTicket(field, ticketSpec);
				});
			}),
		allTickets.end());

	return allTickets;
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

long part2(const TicketSpec &ticketSpec) {
	auto validTickets = getValidTickets(ticketSpec);
	std::map<TicketField, std::set<int>> fieldCandidates;
	for (const TicketField &field : ticketSpec.getFields()) {
		for (int i = 0; i < ticketSpec.getFields().size(); i++) {
			bool matches = std::all_of(validTickets.cbegin(), validTickets.cend(), [=](const std::vector<int> &ticket) {
				return field.isValueInRanges(ticket.at(i));
			});
			if (matches) {
				std::set<int> &candidates = fieldCandidates[field];
				candidates.insert(i);
			}
		}
	}

	std::map<TicketField, int> fieldPositions;
	for (auto candidate : fieldCandidates) {
		std::set<int> valid = candidate.second;
		for (auto candidate2 : fieldCandidates) {
			if (candidate.first.getName() == candidate2.first.getName()) {
				continue;
			}
			std::set<int> difference;
			std::set_difference(
				valid.cbegin(),
				valid.cend(),
				candidate2.second.cbegin(),
				candidate2.second.cend(),
				std::inserter(difference, difference.end()));
			// If valid is a subset of candidate, countign it is futile, and going the other direction will be more
			// helpful
			if (!difference.empty()) {
				valid = std::move(difference);
			}
		}
		assert(valid.size() == 1);
		fieldPositions.emplace(candidate.first, *valid.begin());
	}
	long total = 1;
	for (const auto &fieldPosition : fieldPositions) {
		auto fieldName = fieldPosition.first.getName();
		if (fieldName.rfind("departure", 0) != 0) {
			continue;
		}

		total *= ticketSpec.getOurTicket().at(fieldPosition.second);
	}

	return total;
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

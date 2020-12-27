#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/hana/any_of.hpp>
#include <boost/hana/ext/std/tuple.hpp>
#include <boost/hana/for_each.hpp>
#include <fstream>
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <tuple>

constexpr int P1_NUM_CRAB_TURNS = 100;
constexpr int P2_NUM_CRAB_TURNS = 1E7;
constexpr int NUM_P2_CUPS = 1E6;

/**
 * CupGraph represents a graph of cups in the game. Each item points to its counter-clockwise neighbor.
 *
 * This is effectively the same as a double-circular-linked-list, but there are no values stored beyond the indices, so
 * I just left it as a graph :)
 */
class CupGraph {
 public:
	/**
	 * Iterate over the graph
	 */
	class const_iterator {
	 public:
		using difference_type = int;
		using value_type = const int;
		using pointer = value_type *;
		using reference = value_type &;
		using iterator_category = std::input_iterator_tag;
		/**
		 * Create a new iterator that iterates at the graph, starting at the given value.
		 * Note that this is not the first value that will be returned, but the one whose _NEIGHBOR_ will be returned.
		 * @param graph
		 * @param n
		 */
		const_iterator(const CupGraph &graph, int n) : graph(graph), n(n), stop(n) {
		}

		/**
		 * Increment this iterator to point to the neighbor of the one this element points to
		 * @return const_iterator& The new value of the iterator
		 */
		const_iterator &operator++() {
			auto it = this->graph.neighbors.left.find(this->n);
			if (it == this->graph.neighbors.left.end()) {
				throw "Invalid state. Graph does not contain member that is pointed to by another.";
			}

			n = it->second;
			if (n == this->stop) {
				n = END;
			}

			return *this;
		}

		/**
		 * Increment this iterator to point to the neighbor of the one this element points to
		 * @return const_iterator& The previous value of the iterator
		 */
		const_iterator operator++(int) {
			const_iterator res = *this;
			++(*this);
			return res;
		}

		bool operator==(const const_iterator &other) const {
			return this->n == other.n;
		}

		bool operator!=(const const_iterator &other) const {
			return !(*this == other);
		}

		/**
		 * Resolve this iterator. Note that this returns the NEIGHBOR of the element this points to, not the element
		 * itself.
		 * @return int The neighbor of the element this iterator points to
		 */
		int operator*() const {
			auto it = this->graph.neighbors.left.find(this->n);
			if (it == this->graph.neighbors.left.end()) {
				throw "Invalid state; iterator does not point to value in map";
			}

			return it->second;
		}

	 private:
		static constexpr int END = -1;
		friend CupGraph;
		const CupGraph &graph;
		int n;
		int stop;
	};
	/**
	 * Construct a new CupGraph from the given range
	 * @tparam IntIter An iterator that returns integers
	 * @param start The item to start at
	 * @param end The item to end at
	 */
	template <typename IntIter>
	CupGraph(IntIter start, IntIter end) {
		if (start == end) {
			return;
		}

		int counterclockwiseItem = *start;
		for (auto it = start + 1; it != end; ++it) {
			this->neighbors.insert({counterclockwiseItem, *it});
			counterclockwiseItem = *it;
		}

		// Complete the cycle
		this->neighbors.insert({*(end - 1), *start});
	};

	/**
	 * Get an iterator for a cycle starting at the given element. This will be the *LAST* item returned by the range
	 * (i.e. the first item is the neighbor of this item)
	 * @param start The item to start at.
	 * @return std::pair<CupGraph::const_iterator, CupGraph::const_iterator> The start and end iterators.
	 */
	std::pair<CupGraph::const_iterator, CupGraph::const_iterator> cycleRange(int start) const {
		return std::make_pair(
			CupGraph::const_iterator(*this, start), CupGraph::const_iterator(*this, CupGraph::const_iterator::END));
	}

	/**
	 * The same as move, but moves key, key's neighbor, and key's neighbor's neighbor, at once
	 * @param key The key to move
	 * @param dest The destination to move to
	 */
	void move(int key, int dest, int numToMove) {
		auto keyIt = this->neighbors.left.find(key);
		auto destIt = this->neighbors.left.find(dest);
		if (keyIt == this->neighbors.left.end() || destIt == this->neighbors.left.end()) {
			throw std::out_of_range("Item not in map");
		}

		auto counterclockwiseFromKeyIt = this->neighbors.right.find(key);
		if (counterclockwiseFromKeyIt == this->neighbors.right.end()) {
			throw std::invalid_argument("Key has no counterclockwise neighbor");
		}

		auto endOfRangeIt = keyIt;
		for (int i = 1; i < numToMove; i++) {
			endOfRangeIt = this->neighbors.left.find(endOfRangeIt->second);
			if (endOfRangeIt == this->neighbors.left.end()) {
				throw "Invalid state: elements are not linked together";
			}
		}

		int endOfRangeKey = endOfRangeIt->first;
		int clockwiseFromEndOfRangeKey = endOfRangeIt->second;
		int clockwiseFromDest = destIt->second;
		int counterclockwiseFromKey = counterclockwiseFromKeyIt->second;
		// Unfortunately, using replace doesn't work because we expect that each side has exactly one of each element,
		// so doing this shuffle is not possible with replace, without some serious edgecasing.
		this->neighbors.right.erase(key);
		this->neighbors.left.erase(endOfRangeKey);
		this->neighbors.left.erase(dest);

		// Stitch the counterclockwise neighbor to be the one clockwise to the final element in our range
		this->neighbors.left.insert({counterclockwiseFromKey, clockwiseFromEndOfRangeKey});
		// Stitching the final element of our range to have the same neighbor as the destination did
		this->neighbors.left.insert({endOfRangeKey, clockwiseFromDest});
		// Stitch the destination to be adjacent to the key
		this->neighbors.left.insert({dest, key});
	}

	/**
	 * Get the neighbor of this element
	 * @param n The item to get the neighbor
	 * @return int The neighbor of this element
	 */
	int getNext(int n) {
		auto it = this->neighbors.left.find(n);
		if (it == this->neighbors.left.end()) {
			throw std::out_of_range("No item to get next of");
		}

		return it->second;
	}

 private:
	typedef boost::bimap<boost::bimaps::unordered_set_of<int>, boost::bimaps::unordered_set_of<int>> neighbor_map_type;
	neighbor_map_type neighbors;
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
 * Make a vector of the cups on the given input line
 * @param inputLine The single-line puzzle input
 * @return std::vector<int> The cups on the single line
 */
std::vector<int> makeCupList(const std::string &inputLine) {
	std::vector<int> cups;
	std::transform(inputLine.cbegin(), inputLine.cend(), std::back_inserter(cups), [](char rawNum) {
		if (rawNum < '0' || rawNum > '9') {
			throw std::invalid_argument("Not numeric");
		}

		return rawNum - '0';
	});

	return cups;
}

/**
 * Get the next cups to pick up
 * @param currentCup The current cup
 * @param graph The graph of cups
 * @return std::tuple<int, int, int> The three cups to pick up
 */
std::tuple<int, int, int> getPickedUpCups(int currentCup, const CupGraph &graph) {
	auto range = graph.cycleRange(currentCup);
	auto it = range.first;
	// This may look stupid, but putting *(it++) into make_tuple will not be evaluated in a deterministic order!
	int value1 = *(it++);
	int value2 = *(it++);
	int value3 = *it;
	return std::make_tuple(value1, value2, value3);
}

/**
 * Check if a tuple contains a certain element
 * @tparam Tuple A Tuple that contains only elements of type ValueType
 * @tparam ValueType The type of the value to check membership of
 * @param tuple The tuple to check
 * @param value The value to check for
 * @return true If the tuple contains this item
 * @return false If the tuple does not contain this item
 */
template <typename Tuple, typename ValueType>
bool tupleContainsItem(Tuple tuple, ValueType value) {
	return boost::hana::any_of(tuple, [value](ValueType item) { return item == value; });
}

/**
 * Get the destination cup
 * @param currentCup The current cup
 * @param graph The graph of cups
 * @param minCup The minimum cup in the graph (this is only really passed so we don't have to recompute it every time -
 * it won't change.)
 * @param maxCup The maximum cup in the graph
 * @return int The destination cup for this turn.
 */
int findDestinationCup(int currentCup, const CupGraph &graph, int minCup, int maxCup) {
	std::tuple<int, int, int> pickedUpCups = getPickedUpCups(currentCup, graph);

	int destinationCup = currentCup;
	do {
		destinationCup--;
		if (destinationCup < minCup) {
			destinationCup = maxCup;
		}
	} while (tupleContainsItem(pickedUpCups, destinationCup));

	return destinationCup;
}

/**
 * Get the minimum and maximum element of an iterator. This is similar to std::minmax_element, but it returns the values
 * themselves, not an iterator.
 * @tparam InputIterator The iterator to loop over
 * @tparam IterValue The value type in the iterator
 * @param begin The start of the range
 * @param end The end of the range
 * @return std::pair<IterValue, IterValue> The minimum and maximum in the interator range, as a pair
 */
template <typename InputIterator, typename IterValue>
std::pair<IterValue, IterValue> minmax(InputIterator begin, InputIterator end) {
	if (begin == end) {
		throw std::invalid_argument("No values in range");
	}

	auto res = std::accumulate(
		begin,
		end,
		std::make_pair<std::optional<IterValue>, std::optional<IterValue>>(std::nullopt, std::nullopt),
		[](const std::pair<std::optional<IterValue>, std::optional<IterValue>> &minmaxPair, IterValue value) {
			if (!minmaxPair.first.has_value() && !minmaxPair.first.has_value()) {
				return std::make_pair(value, value);
			}

			// Even though there's an and, we know both items in the pair must have values at this point.
			return std::make_pair(std::min(value, *minmaxPair.first), std::max(value, *minmaxPair.second));
		});

	return std::make_pair(*res.first, *res.second);
}

/**
 * Run an element of the crab's game. This will mutate given current graph.
 * @param startingCup The cup to start at
 * @param graph The graph to run the game against
 * @param numIterations The number of iterators in the game
 */
void runGame(int startingCup, CupGraph &graph, int numIterations) {
	auto graphItems = graph.cycleRange(startingCup);
	std::pair<int, int> cupMinMax = minmax<decltype(graphItems.first), int>(graphItems.first, graphItems.second);
	int currentCup = startingCup;
	for (int i = 0; i < numIterations; i++) {
		int destinationCup = findDestinationCup(currentCup, graph, cupMinMax.first, cupMinMax.second);

		// Since we move three at a time, we only need to get the first picked up cup
		int firstPickedUpCup = graph.getNext(currentCup);
		using tupleType = typename std::result_of<decltype (&getPickedUpCups)(int, const CupGraph &)>::type;
		constexpr auto numCupsToMove = std::tuple_size<tupleType>::value;
		graph.move(firstPickedUpCup, destinationCup, numCupsToMove);

		currentCup = graph.getNext(currentCup);
	}
}

/**
 * Make the cup label for part 1
 * @param graph The cup graph
 * @return std::string The cup label
 */
std::string makeFullCupLabel(const CupGraph &graph) {
	std::string output;
	auto range = graph.cycleRange(1);
	std::transform(range.first, range.second, std::back_inserter(output), [](int cup) -> char { return cup + '0'; });

	output.pop_back();

	return output;
}

std::string part1(const std::string &inputLine) {
	std::vector<int> cups = makeCupList(inputLine);
	CupGraph graph(cups.cbegin(), cups.cend());
	runGame(cups.front(), graph, P1_NUM_CRAB_TURNS);

	return makeFullCupLabel(graph);
}

long part2(const std::string &inputLine) {
	std::vector<int> cups = makeCupList(inputLine);
	// Generate the items needed for part 2. We could maybe do something smart in the graph and generate these
	// as-needed, but it's more complexity than needed. I think I can live without the 4MB of RAM :)
	int required = NUM_P2_CUPS - cups.size();
	cups.reserve(NUM_P2_CUPS);
	int max = *(std::max_element(cups.cbegin(), cups.cend()));
	for (int i = 0; i < required; i++) {
		cups.push_back(max + i + 1);
	}

	CupGraph graph(cups.cbegin(), cups.cend());
	runGame(cups.front(), graph, P2_NUM_CRAB_TURNS);

	auto cycleRange = graph.cycleRange(1);
	long n1 = *(cycleRange.first++);
	long n2 = *cycleRange.first;

	return n1 * n2;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::cerr << argv[0] << " <input_file>" << std::endl;
		return 1;
	}

	auto input = readInput(argv[1]);
	std::cout << part1(input.at(0)) << std::endl;
	std::cout << part2(input.at(0)) << std::endl;
}

#ifndef COMMON_HPP
#define COMMON_HPP

#include <map>
#include <numeric>
#include <tuple>

constexpr char ALIVE_CHAR = '#';
constexpr char DEAD_CHAR = '.';
constexpr int CYCLE_COUNT = 6;

enum CellState { ALIVE, DEAD };

/**
 * Get the minimum and maximum position where an alive cell exist
 * @tparam Position The type of position to use
 * @tparam componentIndex The index in a position tuple that should be min/maxed
 * @param board The board to check
 * @return std::pair<int, int> The minimum/maximum along the component
 */
template <typename PositionTuple, int componentIndex>
std::pair<int, int> getMinMaxAlivePositionsForComponent(const std::map<PositionTuple, CellState> &board) {
	std::vector<std::pair<PositionTuple, CellState>> alivePositionTuples;
	std::copy_if(
		board.cbegin(),
		board.cend(),
		std::back_inserter(alivePositionTuples),
		[](const std::pair<PositionTuple, CellState> &item) { return item.second == ALIVE; });

	auto elements = std::minmax_element(
		alivePositionTuples.cbegin(),
		alivePositionTuples.cend(),
		[](const std::pair<PositionTuple, CellState> &item, const std::pair<PositionTuple, CellState> &item2) {
			return std::get<componentIndex>(item.first) < std::get<componentIndex>(item2.first);
		});
	int minComponent = std::get<componentIndex>(elements.first->first);
	int maxComponent = std::get<componentIndex>(elements.second->first);

	return std::pair<int, int>(minComponent, maxComponent);
}

template <typename Key, typename Val>
Val getOrDefault(const std::map<Key, Val> &map, Val defaultValue, Key key) {
	if (map.find(key) == map.end()) {
		return defaultValue;
	}

	return map.at(key);
}

#endif

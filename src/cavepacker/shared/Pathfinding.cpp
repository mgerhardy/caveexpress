#include "Pathfinding.h"

#include <unordered_set>
#include <algorithm>
#include <queue>
#include <functional>
#include <SDL.h>

namespace cavepacker {

struct Node {
	int index = { -1 };
	int g = { 0 };
	int f = { 0 };
	int predecessor = { -1 };

	inline bool operator<(const Node& rhs) const {
		return f < rhs.f;
	}
};

bool operator==(const Node& node1, const Node& node2) {
	return node1.index == node2.index;
}

bool operator!=(const Node& node1, const Node& node2) {
	return node1.index != node2.index;
}

}

namespace std {
template<> struct hash<cavepacker::Node> {
	inline size_t operator()(const cavepacker::Node& c) const {
		return static_cast<size_t>(c.index);
	}
};
}

namespace cavepacker {

class NodeList {
private:
	typedef std::vector<Node> OpenType;
	typedef std::vector<bool> ClosedType;
	OpenType _openList;
	// TODO: remove me - use costs
	ClosedType _closedList;

	struct Sort {
		inline bool operator()(const Node& node1, const Node& node2) const {
			return node1.f <= node2.f;
		}
	};

	typedef std::pair<int, Node> PQElement;
	typedef std::priority_queue<PQElement, std::vector<PQElement>, std::greater<PQElement>> PQ;
	PQ _priorityQueue;

	int _maxSize;

public:
	typedef Node* iterator;
	typedef const Node* const_iterator;

	NodeList(int maxSize) : _maxSize(maxSize) {
		_openList.reserve(maxSize);
		_openList.assign(maxSize, Node());
		_closedList.reserve(maxSize);
		_closedList.assign(maxSize, false);
	}

	inline void insert(const Node& node) {
		SDL_assert(node.index > 0 && node.index < _maxSize);
		_openList[node.index] = node;
		_priorityQueue.emplace(node.f, node);
	}

	inline Node pop() {
		Node node = _priorityQueue.top().second;
		SDL_assert(node.index > 0 && node.index < _maxSize);
		_priorityQueue.pop();
		_openList[node.index].index = -1;
		return node;
	}

	// TODO: change me - use costs from openlist
	inline bool isClosed(const Node& node) const {
		SDL_assert(node.index > 0 && node.index < _maxSize);
		return _closedList[node.index];
	}

	// TODO: remove me - use costs
	inline void close(const Node& node) {
		SDL_assert(node.index > 0 && node.index < _maxSize);
		_closedList[node.index] = true;
	}

	inline iterator find(const Node& node) {
		SDL_assert(node.index > 0 && node.index < _maxSize);
		Node& storedNode = _openList[node.index];
		if (storedNode.index == node.index) {
			return &storedNode;
		}
		return nullptr;
	}

	inline bool empty() const {
		return _priorityQueue.empty();
	}

	inline const_iterator end() const {
		return nullptr;
	}

	inline int getPredecessor(int index) const {
		return _openList[index].predecessor;
	}
};

static inline void expandNode(const BoardState& state, NodeList& nodelist,
		const Node& currentNode, const Node& endNode) {
	std::vector<int> successors;
	state.getReachableIndices(currentNode.index, successors);
	for (int s : successors) {
		Node successor;
		successor.index = s;
		successor.g = successor.f = 0;
		successor.predecessor = -1;
		if (nodelist.isClosed(successor)) {
			continue;
		}
		int g = currentNode.g + 1;
		NodeList::iterator iter = nodelist.find(successor);
		if (iter != nodelist.end()) {
			if (g >= iter->g)
				continue;
			successor = *iter;
		}
		successor.predecessor = currentNode.index;
		successor.g = g;
		int cc;
		int cr;
		if (!state.getColRowFromIndex(currentNode.index, cc, cr))
			continue;
		int ec;
		int er;
		if (!state.getColRowFromIndex(endNode.index, ec, er))
			continue;
		const int h = std::abs(cc - ec) + std::abs(cr - er);
		successor.f = g + h;
		nodelist.insert(successor);
	}
}

bool astar(const BoardState& bstate, int startIndex, int endIndex, std::vector<int>& path) {
	if (!bstate.isFree(endIndex))
		return false;
	NodeList nodelist(bstate.getWidth() * bstate.getHeight());
	Node startNode;
	startNode.index = startIndex;
	Node endNode;
	endNode.index = endIndex;
	nodelist.insert(startNode);
	SDL_assert(!nodelist.empty());
	do {
		Node currentNode = nodelist.pop();
		if (currentNode == endNode) {
			int current = endIndex;
			path.push_back(current);
			while (current != startIndex) {
				current = nodelist.getPredecessor(current);
				path.push_back(current);
			}
			std::reverse(path.begin(), path.end());
			return true;
		}
		nodelist.close(currentNode);
		expandNode(bstate, nodelist, currentNode, endNode);
	} while (!nodelist.empty());
	return false;
}

}

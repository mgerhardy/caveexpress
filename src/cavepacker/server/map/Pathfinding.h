#pragma once

#include "BoardState.h"

#include <unordered_set>
#include <algorithm>
#include <queue>
#include <vector>

#ifdef DEBUG
#define ASTARDEBUG
#endif

namespace cavepacker {

struct Node {
	int index = { -1 };
	int g = { 0 };
	int f = { 0 };
	int predecessor = { -1 };
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
	ClosedType _closedList;

	struct Sort {
		inline bool operator()(const Node& node1, const Node& node2) const {
			return node1.f <= node2.f;
		}
	};
	std::set<Node, Sort> _priorityQueue;

	int _size;
public:
	typedef Node* iterator;
	typedef const Node* const_iterator;

	NodeList(int maxSize) :
			_size(0) {
		_openList.reserve(maxSize);
		_closedList.reserve(maxSize);
	}

	inline void insert(const Node& node) {
		SDL_assert(node.index > 0);
		_openList[node.index] = node;
		auto i = _priorityQueue.find(node);
		if (i != _priorityQueue.end())
			_priorityQueue.erase(node);
		else
			++_size;
		_priorityQueue.insert(node);
		SDL_assert(_priorityQueue.size() == _size);
	}

	inline Node pop() {
		auto pqi = _priorityQueue.begin();
		Node node = *pqi;
		Log::error(LOG_SERVER, "node %i popped with f: %i",node.index,node.f);
		_priorityQueue.erase(pqi);
		_openList[node.index].index = -1;
		--_size;
		SDL_assert(_priorityQueue.size() == _size);
		return node;
	}

	inline bool isClosed(const Node& node) const {
		SDL_assert(node.index > 0);
		return _closedList[node.index];
	}

	inline void close(const Node& node) {
		SDL_assert(node.index > 0);
		_closedList[node.index] = true;
	}

	inline iterator find(const Node& node) {
		SDL_assert(node.index > 0);
		Node& storedNode = _openList[node.index];
		if (storedNode.index == node.index) {
			return &storedNode;
		}
		return nullptr;
	}

	inline bool empty() const {
		return _size == 0;
	}

	inline const_iterator end() const {
		return nullptr;
	}
};

static inline void expandNode(BoardState& state, NodeList& nodelist,
		const Node& currentNode, const Node& endNode) {
	std::vector<int> successors;
	state.getReachableIndices(currentNode.index, successors);
	for (int s : successors) {
		Node successor;
		successor.index = s;
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
		state.getColRowFromIndex(currentNode.index, cc, cr);
		int ec;
		int er;
		state.getColRowFromIndex(endNode.index, ec, er);
		const int xd = cc - ec;
		const int yd = cr - er;
		const int h = xd * xd + yd * yd;
		successor.f = g + h;
		nodelist.insert(successor);
	}
}

bool astar(BoardState& bstate, int startIndex, int endIndex) {
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
		if (currentNode == endNode)
			return true;
		nodelist.close(currentNode);
#ifdef ASTARDEBUG
		bstate.clearFieldForIndex(currentNode.index);
		bstate.setFieldForIndex(currentNode.index, Sokoban::VISITED);
		Log::debug(LOG_SERVER, "pathfinding state:\n%s", bstate.toString().c_str());
#endif
		expandNode(bstate, nodelist, currentNode, endNode);
	} while (!nodelist.empty());
	return false;
}

}

/*
	File: HashMap.hpp
	Author: Philip Haynes
	A map implemented as a hash table.
	Requires u8 HashMapHash(T) to be defined for T.
*/

#ifndef AZCORE_HASH_MAP_HPP
#define AZCORE_HASH_MAP_HPP

#include "../basictypes.hpp"
#include "Array.hpp"
#include "IndexHash.hpp"
#include <utility>
#include <initializer_list>
#include <stdexcept>

namespace AzCore {

template <typename Node_t, u16 arraySize>
class HashMapIterator;

template <typename Key_t, typename Value_t, u16 arraySize=256>
struct HashMap {
	struct Node {
		Key_t key;
		Value_t value;
		Node *next = nullptr;

		Node() = default;
		Node(const Node &other) : key(other.key), value(other.value) {
			if (other.next) {
				next = new Node(*other.next);
			}
		}
		Node(Node &&other) :
			key(std::move(other.key)), value(std::move(other.value)),
			next(other.next)
		{
			other.next = nullptr;
		}
		Node(Key_t newKey, const Value_t &newValue) :
			key(newKey), value(newValue) {}
		Node(Key_t newKey, Value_t &&newValue) :
			key(newKey), value(std::move(newValue)) {}
		Node(Key_t newKey) : key(newKey), value() {}
		~Node() {
			if (next) delete next;
		}

		Node& operator=(const Node &other) {
			key = other.key;
			value = other.value;
			if (next) {
				if (other.next) {
					*next = *other.next;
				} else {
					delete next;
					next = nullptr;
				}
			} else {
				if (other.next) {
					next = new Node(*other.next);
				}
			}
			return *this;
		}

		Node& operator=(Node &&other) {
			if (next) delete next;
			key = std::move(other.key);
			value = std::move(other.value);
			next = other.next;
			other.next = nullptr;
			return *this;
		}

		Value_t& Emplace(Node && node) {
			if (key == node.key) {
				return value = std::move(node.value);
			} else {
				if (next) {
					return next->Emplace(std::move(node));
				} else {
					next = new Node(std::move(node));
					return next->value;
				}
			}
		}

		bool Exists(const Key_t& testKey) const {
			if (key == testKey) return true;
			if (next) {
				return next->Exists(testKey);
			} else {
				return false;
			}
		}

		Node* Find(const Key_t& testKey) {
			if (key == testKey) return this;
			if (next) {
				return next->Find(testKey);
			} else {
				return nullptr;
			}
		}

		const Node* Find(const Key_t& testKey) const {
			if (key == testKey) return this;
			if (next) {
				return next->Find(testKey);
			} else {
				return nullptr;
			}
		}

		Value_t& ValueOf(const Key_t& testKey, const Value_t &valueDefault=Value_t()) {
			if (key == testKey) return value;
			if (next) {
				return next->ValueOf(testKey, valueDefault);
			} else {
				next = new Node(testKey, valueDefault);
				return next->value;
			}
		}

		Value_t& ValueOf(const Key_t& testKey, Value_t &&valueDefault) {
			if (key == testKey) return value;
			if (next) {
				return next->ValueOf(testKey, std::move(valueDefault));
			} else {
				next = new Node(testKey, std::move(valueDefault));
				return next->value;
			}
		}
	};
	Array<Node*> nodes;

	force_inline() HashMap() : nodes(arraySize, nullptr) {}

	force_inline() HashMap(const HashMap& other) : nodes(arraySize) {
		for (i32 i = 0; i < arraySize; i++) {
			if (other.nodes[i]) {
				nodes[i] = new Node(*other.nodes[i]);
			} else {
				nodes[i] = nullptr;
			}
		}
	}
	force_inline() HashMap(HashMap&& other) : nodes(std::move(other.nodes)) {
		other.nodes.Resize(arraySize, nullptr);
	}

	HashMap(const std::initializer_list<Node> &init) :
	nodes(arraySize, nullptr) {
		for (const Node &node : init) {
			Emplace(node.key, node.value);
		}
	}

	~HashMap() {
		Clear();
	}

	void Clear() {
		for (i32 i = 0; i < nodes.size; i++) {
			if (nodes[i]) {
				delete nodes[i];
				nodes[i] = nullptr;
			}
		}
	}

	HashMap& operator=(const HashMap &other) {
		if (this == &other) return *this;
		Clear();
		for (i32 i = 0; i < arraySize; i++) {
			if (other.nodes[i]) {
				nodes[i] = new Node(*other.nodes[i]);
			} else {
				nodes[i] = nullptr;
			}
		}
		return *this;
	}
	HashMap& operator=(HashMap &&other) {
		if (this == &other) return *this;
		Clear();
		nodes = std::move(other.nodes);
		other.nodes.Resize(arraySize, nullptr);
		return *this;
	}

	force_inline(Value_t&)
	Emplace(Node &&node) {
		return Emplace(node.key, node.value);
	}

	Value_t& Emplace(Key_t key, Value_t value) {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index]) {
			return nodes[index]->Emplace(Node(key, value));
		} else {
			nodes[index] = new Node(key, value);
			return nodes[index]->value;
		}
	}

	bool Exists(Key_t key) const {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index] == nullptr) return false;
		return nodes[index]->Exists(key);
	}

	Node* Find(Key_t key) {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index] == nullptr) return nullptr;
		return nodes[index]->Find(key);
	}

	const Node* Find(Key_t key) const {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index] == nullptr) return nullptr;
		return nodes[index]->Find(key);
	}

	Value_t& ValueOf(Key_t key, const Value_t &valueDefault) {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index]) {
			return nodes[index]->ValueOf(key, valueDefault);
		} else {
			nodes[index] = new Node(key, valueDefault);
			return nodes[index]->value;
		}
	}

	Value_t& ValueOf(Key_t key, Value_t &&valueDefault=Value_t()) {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index]) {
			return nodes[index]->ValueOf(key, std::move(valueDefault));
		} else {
			nodes[index] = new Node(key, std::move(valueDefault));
			return nodes[index]->value;
		}
	}

	const Value_t& ValueOf(Key_t key) const {
		i32 index = IndexHash<arraySize>(key);
		if (nodes[index]) {
			return nodes[index]->ValueOf(key);
		} else {
			throw std::invalid_argument("key doesn't exist in HashMap");
		}
	}

	force_inline(Value_t&)
	operator[](Key_t key) {
		return ValueOf(key);
	}

	force_inline(const Value_t&)
	operator[](Key_t key) const {
		return ValueOf(key);
	}

	HashMapIterator<Node, arraySize> begin() {
		i32 firstIndex = 0;
		while (firstIndex < arraySize && nodes[firstIndex] == nullptr) {
			firstIndex++;
		}
		if (firstIndex == arraySize) {
			return HashMapIterator<Node, arraySize>(&nodes, firstIndex, nullptr);
		} else {
			return HashMapIterator<Node, arraySize>(&nodes, firstIndex, nodes[firstIndex]);
		}
	}
	HashMapIterator<Node, arraySize> end() {
		return HashMapIterator<Node, arraySize>(nullptr, arraySize, nullptr);
	}
	HashMapIterator<const Node, arraySize> begin() const {
		i32 firstIndex = 0;
		while (firstIndex < arraySize && nodes[firstIndex] == nullptr) {
			firstIndex++;
		}
		if (firstIndex == arraySize) {
			return HashMapIterator<const Node, arraySize>((const Array<const Node*>*)&nodes, firstIndex, nullptr);
		} else {
			return HashMapIterator<const Node, arraySize>((const Array<const Node*>*)&nodes, firstIndex, nodes[firstIndex]);
		}
	}
	HashMapIterator<const Node, arraySize> end() const {
		return HashMapIterator<const Node, arraySize>(nullptr, arraySize, nullptr);
	}
};

template <typename Node_t, u16 arraySize>
class HashMapIterator {
	const Array<Node_t*> *nodes;
	i32 index;
	Node_t *node;

public:
	HashMapIterator(const Array<Node_t*> *nodesArray, i32 i, Node_t *n) :
	nodes(nodesArray), index(i), node(n) {}
	void Rebase(const Array<Node_t*> *nodesArray) {
		nodes = nodesArray;
	}
	force_inline(bool)
	operator!=(const HashMapIterator &other) const {
		return node != other.node;
	}
	inline void
	operator++() {
		node = node->next;
		while (node == nullptr) {
			index++;
			if (index < arraySize) {
				node = (*nodes)[index];
			} else {
				break;
			}
		}
	}
	force_inline(Node_t&)
	operator*() {
		return *node;
	}
};

} // namespace AzCore

#endif // AZCORE_HASH_MAP_HPP

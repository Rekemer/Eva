#pragma once
#include "Tokens.h"
#include "String.hpp"
#include <memory>
#include <vector>
#include "HashTable.h"





struct Node
{
	TokenType type;
	int line = 0;
	int childrenCount = 0;
	Node() = default;
	Node(const Node&) = delete;
	Node(Node&&) = default;
	/*Node::Node(const Node& e)
	{
		childrenCount = e.childrenCount;
		type = e.type;
		line = e.line;

	}*/
	virtual ~Node()
	{

	};
	template <typename T>
	const T* As() const
	{
		return static_cast<const T*>(this);
	}
	template <typename T>
	T* AsMut() const
	{
		return (T*)(this);
	}
};

struct Expression : public Node
{
	ValueContainer value;
	int depth = 0;
	Expression() = default;
	Expression(Expression&&);
	Expression(const Expression&) = delete;

	std::unique_ptr<Node> left = nullptr;
	std::unique_ptr<Node> right = nullptr;

};



struct Scope : public Node
{
	std::vector<std::unique_ptr<Node>> expressions;
	HashTable types;
	Scope* prevScope = nullptr;
	StackSim stack;
	int depth = 0;
	Scope() = default;
	// Delete the copy constructor and copy assignment operator
	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;

	// Default the move constructor and move assignment operator
	Scope(Scope&&) = default;
	Scope& operator=(Scope&&) = default;
	int popAmount = 0;

	std::tuple<bool, int> IsLocalExist(String& name, int scopeDepth)
	{
		auto tmpScope = this;
		bool isLocalDeclared = false;
		auto localIndex = -1;
		while (tmpScope != nullptr)
		{
			auto [exist, index] = tmpScope->stack.IsLocalExist(name, scopeDepth);
			if (exist)
			{
				isLocalDeclared = true;
				localIndex = index;
				break;
			}
			tmpScope = tmpScope->prevScope;
		}
		if (isLocalDeclared)
		{
			return { isLocalDeclared,localIndex};
		}
		return { false, - 1 };
	}

	Entry* GetType(String& str)
	{
		Entry* entry = nullptr;
		auto tmpScope = this;
		while (tmpScope != nullptr)
		{
			entry = tmpScope->types.Get(str.GetStringView());
			if (entry->key) break;
			tmpScope = tmpScope->prevScope;
		}
		return entry;
	}

};
struct For : public Node
{
	Scope initScope;
	std::unique_ptr<Node> init = nullptr;
	std::unique_ptr<Node> condition = nullptr;
	std::unique_ptr<Node> action = nullptr;
	std::unique_ptr<Node> body = nullptr;
};
struct FunctionNode : public Node
{
	Scope paramScope;
	std::unique_ptr<Node> body;
	std::shared_ptr<String> name;
	std::vector<std::unique_ptr<Node>> arguments;
};
struct Call : public Node
{
	std::shared_ptr<String> name;
	std::vector<std::unique_ptr<Node>> args;
};

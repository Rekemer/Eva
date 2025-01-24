#pragma once
#include "Tokens.h"
#include <string>
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
	// total
	int popAmount = 0;
	// so far during code gen
	// we can face continue or break
	// total amount will overestimate 
	int currentPopAmount = 0;
	void AddLocal(std::string& name, int currentScopeDepth)
	{
		auto endIterator = stack.locals.begin() + stack.m_StackPtr;
		auto iter = std::find_if(stack.locals.begin(), endIterator, [&](auto& local)
			{
				return local.name == name && local.depth == currentScopeDepth;
			});

		if (iter != endIterator)
		{
			assert(false && "variable already declared in current scope");
		}
		stack.locals[stack.m_StackPtr].name = name;
		stack.locals[stack.m_StackPtr++].depth = currentScopeDepth;
	}
	// returns whether exists, local index, and depth of declaration
	std::tuple<bool, int, int > IsLocalExist(const std::string& name, int scopeDepth) const
	{
		auto tmpScope = this;
		bool isLocalDeclared = false;
		auto localIndex = -1;
		auto currentScope = scopeDepth;
		while (tmpScope != nullptr)
		{
			auto [exist, index] = tmpScope->stack.IsLocalExist(name, scopeDepth);
			if (exist)
			{

				isLocalDeclared = true;
				localIndex = index;
				currentScope = tmpScope->depth; 
				break;
			}
			
			tmpScope = tmpScope->prevScope;
		}
		if (isLocalDeclared)
		{
			assert(currentScope >= 0);
			return { isLocalDeclared,localIndex,currentScope };
		}
		return { false, - 1, 0 };
	}

	ValueType GetType(const std::string& str) const
	{
		Entry* entry = nullptr;
		auto tmpScope = this;
		while (tmpScope != nullptr)
		{
			entry = tmpScope->types.Get(str);
			if (entry->IsInit()) break;
			tmpScope = tmpScope->prevScope;
		}
		if (entry->IsInit())
		{
			return entry->value.type;
		}
		return ValueType::NIL;
	}

	void AddType(const std::string& name, ValueType type)
	{
		types.Add(name,type);
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
	std::string name;
	std::vector<std::unique_ptr<Node>> arguments;
};
struct Call : public Node
{
	// can store return type?
	std::string name;
	std::vector<std::unique_ptr<Node>> args;
	bool isNative = false;
};

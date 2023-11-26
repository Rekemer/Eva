#pragma once
#include <vector>
#include <stack>
#include <cstdlib>
#include "Value.h"
#include "HashTable.h"


class AST;
class Expression;
class VirtualMachine
{
public:
	void Execute();
	void GenerateBytecode(const std::vector<AST>& trees);
	const std::stack<ValueContainer>& GetStack() { return vmStack; };
	Object* AllocateString(const char* ptr, size_t size);
	~VirtualMachine();
private:
	ValueType Generate(const Expression* tree);
	bool AreEqual(const ValueContainer& a, const ValueContainer& b);
private:
	std::vector< uint8_t> opCode;
	std::vector<ValueContainer> constants;
	std::stack<ValueContainer> vmStack;
	HashTable internalStrings;
};
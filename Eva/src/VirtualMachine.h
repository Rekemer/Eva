#pragma once
#include <vector>
#include <stack>
#include <array>
#include <cstdlib>
#include "Value.h"
#include "HashTable.h"

using Bytecode = uint8_t;

struct Local
{
	int depth;
	String name;
};

class AST;
struct Node;
class VirtualMachine
{
public:
	VirtualMachine() = default;
	VirtualMachine(const VirtualMachine& vm)
	{
		internalStrings = vm.internalStrings;
		globalVariables = vm.globalVariables;
		globalVariablesTypes = vm.globalVariablesTypes;
		vmStack= vm.vmStack;

	}
	void Execute();
	void GenerateBytecode(const std::vector<AST>& trees);
	const std::stack<ValueContainer>& GetStack() { return vmStack; };
	Object* AllocateString(const char* ptr, size_t size);

	inline void ClearLocals(int currentScope)
	{
		while (currentScope > 0
			&& currentScope > locals[localPtr].depth)
		{
			localPtr--;
		}
	}
	HashTable& GetGlobals() { return globalVariables; };
	HashTable& GetGlobalsType() { return globalVariablesTypes; };
	~VirtualMachine();
private:
	ValueType Generate(const Node* tree);
	bool AreEqual(const ValueContainer& a, const ValueContainer& b);
private:
	friend void Debug(VirtualMachine& vm);
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
	std::stack<ValueContainer> vmStack;
	HashTable internalStrings;
	HashTable globalVariables;
	HashTable globalVariablesTypes;
	std::array<Local, 256> locals;
	int localPtr = 0;
};
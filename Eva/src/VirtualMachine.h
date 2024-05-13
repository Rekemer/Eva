#pragma once
#include <vector>
#include <stack>
#include <cstdlib>
#include "Value.h"
#include "HashTable.h"

using Bytecode = uint8_t;

class AST;
class Expression;
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
	HashTable& GetGlobals() { return globalVariables; };
	HashTable& GetGlobalsType() { return globalVariablesTypes; };
	~VirtualMachine();
private:
	ValueType Generate(const Expression* tree);
	bool AreEqual(const ValueContainer& a, const ValueContainer& b);
private:
	friend void Debug(VirtualMachine& vm);
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
	std::stack<ValueContainer> vmStack;
	HashTable internalStrings;
	HashTable globalVariables;
	HashTable globalVariablesTypes;
};
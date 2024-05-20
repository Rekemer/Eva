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
struct Scope;
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
	const std::vector<ValueContainer>& GetStack() { return vmStack; };
	Object* AllocateString(const char* ptr, size_t size);

	void AddLocal(String& name, int currentScope);
	
	// checks if exists and returns index
	inline std::tuple<bool, int> IsLocalExist(String & name)
	{
		auto temp = localPtr;
		while (temp >= 0)
		{
			if (name == locals[temp].name)
			{
				return { true ,temp};
			}
			temp--;
		}
		return { false ,-1};
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
	std::vector<ValueContainer> vmStack;
	HashTable internalStrings;
	HashTable globalVariables;
	HashTable globalVariablesTypes;
	const Scope* currentScope;
	std::array<Local, 256> locals;
	int localPtr = 0;
};
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
struct Expression;
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
	// checks if  exists in imitation of runtime stack and returns index
	std::tuple<bool, int> IsLocalExist(String& name);
	HashTable& GetGlobals() { return globalVariables; };
	HashTable& GetGlobalsType() { return globalVariablesTypes; };
	~VirtualMachine();
private:
	void SetVariable(std::vector<Bytecode>& opCode,
		const Expression* expression);
	ValueType GetVariable(std::vector<Bytecode>& opCode, const Expression* expression);
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
	// so we can access previous scopes too
	std::vector<const Scope*> currentScopes;
	std::array<Local, 256> locals;
	// track the declared locals
	int m_StackPtr = 0;
	bool m_Panic = false;
	// if we hit break or continue we should know where to jump
	int m_StartLoopIndex = -1;
	// Once we hit break we should remember the index
	// to come back and  patch it with correct distance
	// for a jump
	int m_BreakIndex= -1;
};
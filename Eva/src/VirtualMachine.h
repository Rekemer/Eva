#pragma once
#include <vector>
#include <stack>
#include <array>
#include <memory>
#include <cstdlib>
#include "Value.h"
#include "Function.h"
#include "HashTable.h"
#include "Bytecode.h"
 



struct CallFrame
{
	Func* function;
	size_t ip = 0;	
	// base pointer to stack
	// which is start of counting
	// for the function 
	int stackIndex = -1;
};
class AST;
struct Node;
struct Block;
struct Expression;
struct Scope;
struct StackSim;
struct Operand;
struct Instruction;
class CFG;
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
	void GenerateBytecodeCFG(const CFG& cfg);
	void GenerateBytecodeAST(const Node const* node);
	const std::vector<ValueContainer>& GetStack() { return vmStack; };
	//std::shared_ptr<String> AllocateString(const char* ptr, size_t size);
	//std::shared_ptr<String> AddStrings(std::shared_ptr<String> s, std::shared_ptr<String> s1);

	void ClearLocal();
	HashTable& GetGlobals() { return globalVariables; };
	HashTable& GetGlobalsType() { return globalVariablesTypes; };
	ValueType GetGlobalType(const std::string& str );
	~VirtualMachine();
private:
	Block* HandleBranch(std::vector<Block*> branches, const Instruction& branch);
	void GenerateBlockInstructions(Block* block);
	ValueType GenerateAST(const Node* tree);
	void GenerateCFG(Block* block);
	void GenerateConstant(const ValueContainer& v);
	void GenerateCFGOperand(const Operand& operand, ValueType instrType);

	void ClearScope(const Scope* scope, StackSim& stackSim,
		std::vector<Bytecode>& opCode);
	void CastWithDeclared(ValueType assignedType, ValueType declared);
	//void CollectStrings();
	void BeginContinue(int startLoopIndex);
	int BeginBreak();
	void EndContinue();
	size_t CallFunction(Func* func, size_t argumentCount, size_t baseIndex);
	void PatchBreak(int prevSizeBreak);
	void SetVariable(std::vector<Bytecode>& opCode,
		const std::string& name, int depth);
	ValueType GetVariable(std::vector<Bytecode>& opCode, const std::string& name, int depth);

	ValueType GetLocalType(const std::string& str );

	ValueType GetVariableType(const std::string& name, int depthOfDeclaration);
	// returns index to be patchd for a jump if loop is finished
	int GenerateLoopCondition(const Node* node);
	bool AreEqual(const ValueContainer& a, const ValueContainer& b);
private:
	// function we build or execute
	Func* currentFunc = nullptr;
	// entry point
	Func* mainFunc = nullptr;
	// to execute global code
	std::unique_ptr<Func> globalFunc = std::make_unique<Func>();
	
	std::vector<ValueContainer> vmStack;
	std::vector<std::string> functionNames;
	HashTable internalStrings;
	HashTable globalVariables;
	HashTable globalVariablesTypes;

	ValueContainer temp;
	
	std::array<CallFrame, 64> callFrames;
	int nextToCurrentCallFrame = 0;
	bool m_Panic = false;
	bool m_IsSameOrder = false;
	// if we hit break or continue we should know where to jump
	// we need stack because there can be loops in loops
	std::stack<int> m_StartLoopIndexes;
	// Once we hit break we should remember the index
	// to come back and  patch it with correct distance
	// for a jump
	std::stack<int> m_BreakIndexes;
	int conditionIndex = -1;
public:
	Scope* currentScope= nullptr;
};
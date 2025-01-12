#pragma once
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include "Value.h"
#include "Function.h"
#include "HashTable.h"
class CFG;
struct Node;
struct Func;
struct Block;
struct Operand;
struct Instruction;
struct CFGFunction;
class Compiler
{
public:
	int Compile(const char* source);
	HashTable& GetGlobals() { return globalVariables; };
	HashTable& GetGlobalsType() { return globalVariablesTypes; };
	ValueType GetGlobalType(const std::string& str);


	std::unordered_map<std::string, ValueContainer> Preprocess(const char* source);

private:



	Block* HandleBranch(std::vector<Block*> branches, const Instruction& instr);

	void BeginContinue(int startLoopIndex);
	void EndContinue();
	int BeginBreak();
	void PatchBreak(int prevSizeBreak);

	void CastWithDeclared(ValueType assignedType, ValueType declared);
	void GenerateCFGOperand(const Operand& operand, ValueType instrType);
	void GenerateConstant(const ValueContainer& v);
	void GenerateBlockInstructions(Block* block);
	void GenerateCFG(Block* block);
	void GenerateBytecodeCFG(const CFG& cfg);
	//void GenerateBytecodeAST(const Node const* node);
private:

	std::vector<std::string> functionNames;
	HashTable globalVariables;
	HashTable globalVariablesTypes;

	const std::unordered_map<std::string, CFGFunction>* functionCFG;
	// function we build or execute
	Func* currentFunc = nullptr;
	// entry point
	Func* mainFunc = nullptr;
	// to execute global code
	std::unique_ptr<Func> globalFunc = std::make_unique<Func>();


	bool m_Panic = false;
	// if we hit break or continue we should know where to jump
	// we need stack because there can be loops in loops
	std::stack<int> m_StartLoopIndexes;
	// Once we hit break we should remember the index
	// to come back and  patch it with correct distance
	// for a jump
	std::stack<int> m_BreakIndexes;
	std::stack<int> conditionIndex;
	ValueType m_FuncReturnType = ValueType::NIL;
	std::stack<ValueType> lastReturnType;

};
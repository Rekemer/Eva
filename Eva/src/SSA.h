#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Tokens.h"
#include <ostream>
#include <set>
#include <functional>
#include <stack>
#include "Value.h"
struct Operand
{
	ValueContainer value;
	int version;
	bool isConstant = false;

	Operand(const ValueContainer& value, bool isConstant, int version) : value{ value},
		version{ -1 },
		isConstant{ isConstant }
	{

	}

	Operand() : version{-1}
	{

	}

	bool isVariable()
	{
		return value.type == ValueType::STRING && version != -1;
	}
};


struct Block;
struct Instruction 
{
	TokenType instrType;
	Operand operLeft;
	Operand operRight;
	Operand result;
	Instruction(TokenType instr, Operand left, Operand right, Operand res) : 
		instrType{ instr }, 
		operLeft{left},
		operRight{ right},
		result{ res}
	{

	}
	// for branches 
	std::vector<Block*> targets;

	// for phi function
	std::vector<Operand> variables;
};


// Straight-Line Code : code that has only one flow of execution (not jumps like if and else)
struct Block
{
	std::string name;
	
	std::vector<Instruction> instructions;
	std::vector<int> phiInstructionIndexes;
	
	// for building dominator trees
	std::vector<Block*> parents;
	// children in dominator tree
	std::vector<Block*> dominatorChildren;

	// blocks that dominate current block
	std::set<Block*> dom;

	// immediate dominator
	Block* idom = nullptr;

	// dominance frontier
	std::set<Block*> df;

	// next blocks - children
	std::vector<Block*> blocks;
	// number of block - number of node in graph
	static inline int counterStraight = 0;
	static inline int counterThen = 0;
	static inline int counterElse = 0;
	static inline int counterElif = 0;
	static inline int counterMerge = 0;
};

struct Node;
struct Expression;
class Tree;
class VirtualMachine;
// control flow graph
class CFG
{
public:
	VirtualMachine* vm;
	void BuildDominatorTree();
	void ConvertAST(const Node* tree);
	void TopSort();
	void BuildDF();
	void InsertPhi();
	void Debug();
private:
	int NewName(std::string& name);
	void Rename(Block* b);
	void FindDoms();
	void FindIDoms();

	void CreateVariable(const Node* tree);
	Operand ConvertExpressionAST(const Node* tree);
	void ConvertStatementAST(const Node* tree);
	bool IsStatement(const Node* node);
	Block* CreateBlock(const std::string& name, std::vector<Block*>  parents);
	Operand BinaryInstr(const Expression* expr, TokenType type);
	int GetTempVersion ()
	{
		return tempVersion++;
	};


	//breadth first search
	void Bfs(Block* start, std::function<void(Block*)> action);

private:
	bool createBlock = true;
	int tempVersion = 0;
public:
	// whenever we hit condition we create a new block
	Block* currentBlock = nullptr;
	Block* startBlock = nullptr;
private:
	
	// int is a version
	std::unordered_map<std::string, int> variableCounterGlobal;
	std::unordered_map<std::string, int> variableCounterLocal;

	std::unordered_map<std::string, std::vector<Block*>> globalAssigned;
	std::unordered_map<std::string, std::vector<Block*>> localAssigned;


	std::unordered_map<std::string, Block > graph;
	// for renaming stage
	std::unordered_map<std::string, std::stack<int>> variableStack;
	

	// topologically sorted graph
	std::vector<Block*> tpgSort;

};
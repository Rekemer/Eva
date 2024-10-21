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

const int LABEL_VERSION = -3;
const int NOT_INIT_VERSION = -2;
const int NOT_INIT_OPERAND = -1;
const int IS_TEMP = -4;
struct Operand
{
	ValueContainer value;
	int version;
	bool isConstant = false;
	bool isTemp = false;
	// local variable index
	int index = -1;
	int depth = -1;
	ValueType type = ValueType::NIL;
	Operand(const ValueContainer& value, bool isConstant, int version) : value{ value},
		version{ version },
		isConstant{ isConstant }
	{

	}
	Operand() : version{ NOT_INIT_OPERAND}
	{

	}

	bool IsVariable() const
	{
		return value.type == ValueType::STRING && isConstant == false && isTemp == false;
	}
	bool IsTemp()  const
	{
		return isTemp;
	}
};


struct Block;
struct Instruction 
{
	// the value type instruction returns
	ValueType returnType = ValueType::NIL;
	TokenType instrType;
	Operand operLeft;
	Operand operRight;
	Operand result;
	Instruction() = default;
	Instruction(TokenType instr, Operand left, Operand right, Operand res) : 
		instrType{ instr }, 
		operLeft{left},
		operRight{ right},
		result{ res}
	{

	}
	bool IsUnary() const
	{
		return (TokenType::BANG== instrType || TokenType::MINUS == instrType) && operLeft.version == NOT_INIT_OPERAND && operRight.version != NOT_INIT_OPERAND;
	}
	// for branches 
	std::vector<Block*> targets;

	// for phi function and multie branching
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

	bool isVisited = false;
};

struct Node;
struct Scope;
struct Expression;
class Tree;
class VirtualMachine;
// control flow graph
class CFG
{
public:
	CFG()
	{
		startBlock = currentBlock = CreateBlock("[block_0]", {});
	}
	VirtualMachine* vm;
	void BuildDominatorTree();
	void ConvertAST(const Node* tree);
	void TopSort();
	void BuildDF();
	void InsertPhi();
	void Debug();
private:
	Block* CreateBranchBlock(Block* parentBlock, Instruction& branch, Node* flows, const std::string& BlockName);
	int NewName(const std::string& name);
	void Rename(Block* b);
	void FindDoms();
	void FindIDoms();

	Operand CreateTemp();
	void CreateVariable(const Node* tree, TokenType type);
	Operand InitVariable(const std::string& name, int depth);

	void InitLocal(Operand& op,const std::string& name);
	void InitGlobal(Operand& op,const std::string& name);

	void CreateVariableFrom(const Node* tree, const Operand& rightOp);
	Operand ConvertExpressionAST(const Node* tree);
	void ConvertStatementAST(const Node* tree);
	bool IsStatement(const Node* node);
	Block* CreateBlock(const std::string& name, std::vector<Block*>  parents);
	Operand BinaryInstr(const Expression* expr, TokenType type);
	Operand UnaryInstr(const Expression* expr, TokenType type);
	//breadth first search
	void Bfs(Block* start, std::function<void(Block*)> action);

public:
	// whenever we hit condition we create a new block
	Block* currentBlock = nullptr;
	Block* startBlock = nullptr;
	// Scope
	const Scope* currentScope = nullptr; 
private:
	
	// int is a version
	std::unordered_map<std::string, int> variableCounterGlobal;
	std::unordered_map<std::string, int> variableCounterLocal;

	std::unordered_map<std::string, std::vector<Block*>> globalAssigned;
	std::unordered_map<std::string, std::vector<Block*>> localAssigned;

	int tempVersion = 0;
	std::unordered_map<std::string, Block > graph;
	// for renaming stage
	std::unordered_map<std::string, std::stack<int>> variableStack;
	

	// topologically sorted graph
	std::vector<Block*> tpgSort;

};
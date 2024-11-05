#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Tokens.h"
#include <ostream>
#include <set>
#include <queue>
#include <functional>
#include <stack>
#include <format>
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
	std::vector<int> defIndexes;
	Operand(const ValueContainer& value, bool isConstant, int version) : value{ value},
		version{ version },
		isConstant{ isConstant }
	{

	}
	Operand() : version{ NOT_INIT_OPERAND}
	{

	}
	std::string GetTempName()
	{
		return std::format("t{}",version);
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
	bool isMarked = false;
	bool isCritical = false;
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
	
	Block* argBlock;
};

// Custom hash function for std::pair<int, std::string>
struct pair_hash {
	std::size_t operator()(const std::pair<int, std::string>& p) const {
		// Use std::hash to hash both the int and string parts of the pair
		std::size_t h1 = std::hash<int>{}(p.first);
		std::size_t h2 = std::hash<std::string>{}(p.second);

		// Combine the two hashes (simple way to combine hashes)
		return h1 ^ (h2 << 1); // XOR and bit shift to mix the hashes
	}
};
struct pair_hash_block {
	std::size_t operator()(const std::pair<Block*, std::string>& p) const {
		std::size_t h1 = std::hash<Block*>{}(p.first);
		std::size_t h2 = std::hash<std::string>{}(p.second);

		return h1 ^ (h2 << 1);
	}
};
using DefMap = std::unordered_map<std::pair<int, std::string>, std::vector<int>, pair_hash>;
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
	// every path from this to postdom nodes must lead to exit
	std::set<Block*> postdom;

	// immediate dominator
	Block* idom = nullptr;
	// immediate post dominator
	Block* ipdom = nullptr;

	// dominance frontier
	std::set<Block*> df;
	// reverse dominance frontier
	std::set<Block*> rdf;

	DefMap defs;
	std::unordered_map<std::string,std::vector<int>> uses;

	// next blocks - children
	std::vector<Block*> blocks;
	Block* merge; 
	// number of block - number of node in graph
	static inline int counterStraight = 0;
	static inline int counterThen = 0;
	static inline int counterElse = 0;
	static inline int counterElif = 0;
	static inline int counterMerge = 0;
	static inline int counterWhileCondition = 0;
	static inline int counterWhileBody = 0;
	static inline int counterForCondition = 0;
	static inline int counterForBody = 0;
	static inline int counterForAction = 0;
	static inline int counterForInit = 0;

	bool isVisited = false;
	bool isSweeped = false;
	bool markAll = false;
	int offsetPhi = 0;
};


struct CFGFunction
{
	Block* start;
	Block* current;
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
		startBlock = currentBlock = CreateBlock(currentFunc, "[global_start]", {});
	}
	VirtualMachine* vm;
	void BuildDominatorTree();
	void ConvertAST(const Node* tree);
	void TopSort();
	void BuildDF();
	void InsertPhi();
	void DeadCode();
	void Debug();
private:
	void MarkOperand(Block* block, Operand& oper, std::queue<std::pair<Block*, Instruction*>>& workList);
	Instruction CreatePhi(const std::string& name);
	void Sweep(Block* block);
	void Mark(Block* b, std::queue<std::pair<Block*, Instruction*>>& workList);
	void AddDef(int depth, const std::string& name, int index);
	Block* CreateConditionBlock(const std::string& name,  Block* currentBlock);
	Block* CreateBranchBlock(Block* parentBlock, Instruction& branch, Node* flows, const std::string& BlockName, const std::string& mergeName);
	int NewName(const std::string& name);
	void Rename(Block* b);
	void FindDoms();
	void FindIDoms();
	Block* CreateMergeBlock(std::vector<Block*> parents);
	Operand CreateTemp();
	void CreateVariable(const Node* tree, TokenType type);
	Operand InitVariable(const std::string& name, int depth);

	void InitLocal(Operand& op,const std::string& name);
	void InitGlobal(Operand& op,const std::string& name);

	void CreateVariableFrom(const Node* tree, const Operand& rightOp);
	Operand ConvertExpressionAST(const Node* tree);
	void ConvertStatementAST(const Node* tree);
	void EmitPop(Block* currentBlock, int popAmount);
	bool IsStatement(const Node* node);
	Block* CreateBlock(const std::string& currentFunction, const std::string& name, std::vector<Block*>  parents);
	Operand BinaryInstr(const Expression* expr, TokenType type);
	Operand UnaryInstr(const Expression* expr, TokenType type);
	//breadth first search
	void Bfs(Block* start, std::function<void(Block*)> action);

public:
	// whenever we hit condition we create a new block
	Block* currentBlock = nullptr;
	Block* startBlock = nullptr;
	// Scope
	Scope* currentScope = nullptr; 

	std::unordered_map<std::string, CFGFunction> functionCFG;
	std::string currentFunc = "global";
private:
	// int is a version
	std::unordered_map<std::string, int> variableCounterGlobal;
	std::unordered_map<std::string, int> variableCounterLocal;

	std::unordered_map<std::string, std::vector<Block*>> globalAssigned;
	std::unordered_map<std::string, std::vector<Block*>> localAssigned;
	std::stack<int> forDepth;
	int tempVersion = 0;
	std::unordered_map<std::string, Block > graph;
	// for renaming stage
	std::unordered_map<std::string, std::stack<int>> variableStack;
	
	// so we do not pop variables that are already taken care of by the end of loops
	std::unordered_set<std::string> notPoped;
	bool isNotPop = false;

	std::unordered_map < std::pair<Block*, std::string> , std::vector<int >, pair_hash_block> globalDefs;
	// topologically sorted graph
	std::vector<Block*> tpgSort;
	// so we do not pop variables that are already taken care of by the end of loops
	bool getAsParam = false;
	ValueType paramType = ValueType::NIL;
};
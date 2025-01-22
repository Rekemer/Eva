#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Tokens.h"
#include <ostream>
#include <set>
#include <queue>
#include <deque>
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
	std::string originalName;
	Operand(const ValueContainer& value, bool isConstant, int version) : value{ value},
		version{ version },
		isConstant{ isConstant }
	{

	}
	Operand() : version{ NOT_INIT_OPERAND}
	{

	}
	void SetConst()
	{
		isConstant = true;
		isTemp = false;
	}
	std::string GetTempName()
	{
		return std::format("t_{}",version);
	}
	std::string GetVariableVerName()
	{
		return std::format("{}_{}", originalName,version);
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

//If n’s value cannot be known—for example, it is
//defined by reading a value from external media—sscp sets Value(n) to bottom.
//Finally, if n’s value is not known, sscp sets Value(n) to top .If Value(n) is
//not > , the algorithm adds n to the worklist.
enum LatticeValueType { TOP, CONSTANT, BOTTOM };

struct LatticeValue {
	LatticeValueType type = TOP;
	ValueContainer value; // holds actual constant if type == CONSTANT
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
//depth name version inde
using DefMap = std::unordered_map<std::pair<int, std::string>, std::vector<int>, pair_hash>;
using ValueMap = std::unordered_map<std::pair<int, std::string>, ValueContainer, pair_hash>;
using LatticeMap = std::unordered_map<std::pair<int, std::string>, LatticeValue, pair_hash>;
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
	bool updateIndex = false;
	int offsetPhi = 0;
	bool isLoop = false;
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
class Compiler;
// control flow graph
class CFG
{
public:
	CFG()
	{
		startBlock = currentBlock = CreateBlock(currentFunc, "[global_start]", {});
	}
	Compiler* compiler;
	void BuildDominatorTree();
	void ConvertAST(const Node* tree);
	void TopSort();
	void BuildDF();
	void InsertPhi();
	void DeadCode();
	void ConstPropagation();
	void Debug();
private:
	bool UpdateOperand(Operand& op);
	void PropagateTempValues(Block* block, size_t startIndex, Instruction& initialInstr, std::deque<std::pair<int, std::string>>& workList);
	void ConstProp(Block* b, std::deque<std::pair<int, std::string>>& workList);
	void MarkOperand(Block* block, Operand& oper, std::queue<std::pair<Block*, Instruction*>>& workList);
	Instruction CreatePhi(const std::string& name);
	void Sweep(Block* block);
	void Mark(Block* b, std::queue<std::pair<Block*, Instruction*>>& workList);
	
	void AddUse(int depth, const std::string& name, int index, Block* b);
	void AddDef(int depth, const std::string& name, int index, Block* b);
	Block* CreateConditionBlock(const std::string& name,  Block* currentBlock);
	Block* CreateBranchBlock(Block* parentBlock, Instruction& branch, Node* flows, const std::string& BlockName, const std::string& mergeName);

	int NewName(const std::string& name, Block* b);
	
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

	std::unordered_map<std::pair<int, std::string>, int, pair_hash> removedLocal;
	std::unordered_map<int, int> removedLocalTotal;
	std::unordered_map<std::string, bool> isFuncCritical;
	std::stack<bool> parseFunc;
	std::stack<bool> isReturn;
	// int is a version
	std::unordered_map<std::string, int> variableCounterGlobal;
	std::unordered_map<std::string, int> variableCounterLocal;

	std::unordered_map<std::string, std::vector<Block*>> localAssigned;
	std::unordered_map<std::string, std::vector<Block*>> localUses;

	std::unordered_map < std::pair<Block*, std::string> , std::vector<int >, pair_hash_block> globalDefs;
	std::unordered_map < std::pair<Block*, std::string>, std::vector<int >, pair_hash_block>globalUses;

	bool writeToVariable = false;
	
	// we can remove a variable 
	// that stores return variable of a critical fucntion
	// and yet the variable is not used anywhere, hence variable is removed
	// a headache is that we don't know how to pop temp value from stack
	bool isVariableCritical = false;
	std::stack<int> forDepth;
	int tempVersion = 0;
	std::unordered_map<std::string, Block > graph;
	// for renaming stage
	std::unordered_map<std::string, std::stack<std::pair<int, Block*>>> variableStack;
	//std::unordered_map<std::string, std::stack<int>> variableStack;
	
	// so we do not pop variables that are already taken care of by the end of loops
	std::unordered_set<std::string> notPoped;
	bool isNotPop = false;

	// topologically sorted graph
	std::vector<Block*> tpgSort;
	// so we do not pop variables that are already taken care of by the end of loops
	bool getAsParam = false;
	ValueType paramType = ValueType::NIL;
	// Map from variable identifier to its lattice value
	LatticeMap value;

	std::unordered_set<Block*> visitedBlocks;

	std::stack<bool> parseLoop;

};
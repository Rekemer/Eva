#pragma once
#include"String.hpp"
#include <vector>
#include <unordered_map>
#include "Tokens.h"
#include<ostream>
struct Operand
{
	std::string name;
	bool isConstant;
	int version;

	Operand(std::string& name ) : name{ name },
		isConstant{ false },
		version{ -1 }
	{

	}

	Operand() : name{"null"},
		isConstant{false},
		version{-1}
	{

	}

	Operand(const std::string& name, bool isConstant, int version) : 
		name{ name }, 
		isConstant{ isConstant }, 
		version{ version }
	{

	}

	Operand(const char* name, bool isConstant, int version) :
		name{ name },
		isConstant{ isConstant },
		version{ version }
	{

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
};


// Straight-Line Code : code that has only one flow of execution (not jumps like if and else)
struct Block
{
	std::string name;
	
	std::vector<Instruction> instructions;
	
	// next blocks
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
	void ConvertAST(const Node* tree);
	void Debug();
private:
	void CreateVariable(const Node* tree);
	Operand ConvertExpressionAST(const Node* tree);
	void ConvertStatementAST(const Node* tree);
	bool IsStatement(const Node* node);
	Block* CreateBlock(const std::string& name);
	Operand BinaryInstr(const Expression* expr, TokenType type);
	int GetTempVersion ()
	{
		return tempVersion++;
	};


private:
	bool createBlock = true;
	int tempVersion = 0;
	// whenever we hit condition we create a new block
	Block* currentBlock = nullptr;
	Block* startBlock = nullptr;
	// int is a version
	std::unordered_map<String, int> globalVariables;
	std::unordered_map<String, int> localVariables;
	std::unordered_map<std::string, Block > graph;
};
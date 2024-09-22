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
	
	

};

// Straight-Line Code : code that has only one flow of execution (not jumps like if and else)
struct Block
{
	std::string name;
	std::unordered_map<String, int> localVariables;
	std::vector<Instruction> instructions;
	
	// next blocks
	std::vector<Block> blocks;
	// number of block - number of node in graph
	static inline int number = 0;
};

struct Node;
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
	Operand ConvertExpressionAST(const Node* tree);
	void ConvertStatementAST(const Node* tree);
	bool IsStatement(const Node* node);
private:
	int tempVersion = 0;
	// whenever we hit condition we create a new block
	Block* currentBlock;
	// int is a version
	std::unordered_map<String, int> globalVariables;
	std::unordered_map<std::string, Block > graph;
};
#pragma once
#include <vector>
#include <stack>
#include <cstdlib>
#include "Value.h"


class AST;
class Expression;
class VirtualMachine
{
public:
	void Execute();
	void GenerateBytecode(const AST& tree);
	const std::stack<ValueContainer>& GetStack() { return vmStack; };
	~VirtualMachine();
private:
	void Generate(const Expression* tree);
private:
	std::vector< uint8_t> opCode;
	std::vector<float> constants;
	std::stack<ValueContainer> vmStack;
};
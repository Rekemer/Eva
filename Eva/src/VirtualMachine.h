#pragma once
#include <vector>
#include <stack>
#include <cstdlib>
#include "Value.h"


class Expression;
class VirtualMachine
{
public:
	void Execute();
	void GenerateBytecode(Expression* tree);
	const std::stack<ValueContainer>& GetStack() { return vmStack; };
	~VirtualMachine();
private:
	void Generate(Expression* tree);
private:
	std::vector< uint8_t> opCode;
	std::vector<float> constants;
	std::stack<ValueContainer> vmStack;
};
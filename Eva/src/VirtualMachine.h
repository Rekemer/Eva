#pragma once
#include <vector>
#include <stack>
#include <cstdlib>

enum  InCode
{
	CONST_VALUE = 1,
	ADD = 2,
	DIVIDE = 3,
	MULTIPLY = 4,
	SUBSTRACT = 5,
	NEGATE = 6,
	RETURN = 7,

};

class Expression;
class VirtualMachine
{
public:
	void Execute();
	void GenerateBytecode(Expression* tree);
	const std::stack<float>& GetStack() { return vmStack; };
	~VirtualMachine();
private:
	void Generate(Expression* tree);
private:
	std::vector< uint8_t> opCode;
	std::vector<float> constants;
	std::stack<float> vmStack;
};
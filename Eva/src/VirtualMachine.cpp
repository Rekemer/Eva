#include "VirtualMachine.h"
#include "Expression.h"
#define BINARY_OP(operation)\
{\
auto v = vmStack.top();\
vmStack.pop();\
auto v2 = vmStack.top();\
vmStack.pop();\
vmStack.push(v2 operation v);\
++ipIndex;\
}\
while(false)

void VirtualMachine::Generate(Expression * tree)
{
		if (!tree) return;

		if (tree->type == TokenType::PLUS)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::ADD);
		}
		else if (tree->type == TokenType::STAR)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::MULTIPLY);
		}
		else if (tree->type == TokenType::MINUS)
		{
			Generate(tree->left);

			if (tree->right)
			{
				Generate(tree->right);
				opCode.push_back((uint8_t)InCode::SUBSTRACT);
			}
			else
			{
				opCode.push_back((uint8_t)InCode::NEGATE);
			}

		}
		else if (tree->type == TokenType::SLASH)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::DIVIDE);
		}
		else if (tree->type == TokenType::NUMBER)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.push_back(tree->value);
			opCode.push_back(constants.size() - 1);
		}
		else
		{
			
		}

}

VirtualMachine::~VirtualMachine()
{
}



void VirtualMachine::Execute()
{
	int ipIndex = 0;

	while (true)
	{
		auto inst = opCode[ipIndex];
		switch (inst)
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push(constants[opCode[++ipIndex]]);
			++ipIndex;
			break;
		}
		case InCode::ADD:
		{
			BINARY_OP(+);
			break;
		}
		case InCode::SUBSTRACT:
		{
			BINARY_OP(-);
			break;
		}
		case InCode::MULTIPLY:
		{
			BINARY_OP(*);
			break;
		}
		case InCode::DIVIDE:
		{
			BINARY_OP(/ );
			break;
		}
		case InCode::NEGATE:
		{
			auto value = vmStack.top();
			vmStack.pop();
			vmStack.push(-value);
			++ipIndex;
			break;
		}
		case InCode::RETURN:
		{
			return;
		}
		default:
			break;
		}
	}
}

void VirtualMachine::GenerateBytecode(Expression* tree)
{
	Generate(tree);
	opCode.push_back((uint8_t)InCode::RETURN);
}

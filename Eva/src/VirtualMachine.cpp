#include "VirtualMachine.h"
#include "Expression.h"
#include "String.hpp"
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.top().as.type;\
vmStack.pop();\
auto v2 = vmStack.top().as.type;\
vmStack.pop();\
vmStack.push(ValueContainer{v2 operation v});\
}\
while(false)
enum  InCode
{
	CONST_VALUE,
	TRUE,
	FALSE,
	NIL,
	ADD,
	DIVIDE,
	MULTIPLY,
	SUBSTRACT,
	NEGATE,
	GREATER,
	LESS,
	EQUAL_EQUAL,
	NOT,
	RETURN,

};

void VirtualMachine::Generate(const Expression * tree)
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
			constants.push_back(ValueContainer{ tree->value.as.numberFloat });
			opCode.push_back(constants.size() - 1);
		}
		else if (tree->type == TokenType::STRING)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.emplace_back(tree->value.as.object );
			opCode.push_back(constants.size() - 1);
		}

		else if (tree->type == TokenType::TRUE)
		{
			opCode.push_back((uint8_t)InCode::TRUE);
		}
		else if (tree->type == TokenType::FALSE)
		{
			opCode.push_back((uint8_t)InCode::FALSE);
		}
		else if (tree->type == TokenType::GREATER)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::GREATER);
		}
		else if (tree->type == TokenType::GREATER_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::LESS);
			opCode.push_back((uint8_t)InCode::NOT);
		}
		else if (tree->type == TokenType::EQUAL_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
		}
		else if (tree->type == TokenType::LESS_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::GREATER);
			opCode.push_back((uint8_t)InCode::NOT);
		}
		else if (tree->type == TokenType::LESS)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::LESS);
		}
		else if (tree->type == TokenType::BANG)
		{
			Generate(tree->left);
			opCode.push_back((uint8_t)InCode::NOT);

		}

}

VirtualMachine::~VirtualMachine()
{
}


bool VirtualMachine::AreEqual(const ValueContainer& a, const ValueContainer& b)
{
	if (a.type == b.type && a.type == ValueType::BOOL)
	{
		return a.as.boolean == b.as.boolean;
	}
	else if (a.type == b.type && a.type == ValueType::FLOAT)
	{
		return fabs(a.as.numberFloat - b.as.numberFloat) < 0.04;
	}
	else if (a.type == b.type && a.type == ValueType::OBJ)
	{
		auto str = dynamic_cast<String*>(a.as.object);
		auto str2 = dynamic_cast<String*>(b.as.object);
		return *str == *str2;
	}
	return false;
}
void VirtualMachine::Execute()
{
	int ipIndex = 0;

	while (true)
	{
		auto inst = opCode[ipIndex++];
		switch (inst)
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push(std::move(constants[opCode[ipIndex++]]));
			break;
		}
		case InCode::TRUE:
		{
			vmStack.push(ValueContainer{ true });
			break;
		}
		case InCode::FALSE:
		{
			vmStack.push(ValueContainer{ false });
			break;
		}
		case InCode::ADD:
		{
			BINARY_OP(numberFloat, +);
			break;
		}
		case InCode::SUBSTRACT:
		{
			BINARY_OP(numberFloat, -);
			break;
		}
		case InCode::MULTIPLY:
		{
			BINARY_OP(numberFloat,*);
			break;
		}
		case InCode::DIVIDE:
		{
			BINARY_OP(numberFloat ,/ );
			break;
		}
		case InCode::NEGATE:
		{
			auto value = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{-value.as.numberFloat});
			break;
		}
		case InCode::NOT:
		{
			auto value = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{ !value.as.boolean });
			break;
		}
		case InCode::LESS:
		{
			BINARY_OP(numberFloat, < );
			break;
		}
		case InCode::GREATER:
		{
			BINARY_OP(numberFloat, > );
			break;
		}
		case InCode::EQUAL_EQUAL:
		{
			auto&  v2 = vmStack.top();
			vmStack.pop();
			auto&  v1 = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{ AreEqual(v1,v2) });
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

void VirtualMachine::GenerateBytecode(const AST& tree)
{
	Generate(tree.GetTree());
	opCode.push_back((uint8_t)InCode::RETURN);
}

#include "VirtualMachine.h"
#include "Expression.h"
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.top().as.type;\
vmStack.pop();\
auto v2 = vmStack.top().as.type;\
vmStack.pop();\
vmStack.push(ValueContainer{v2 operation v});\
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
			constants.push_back(tree->value.as.numberFloat);
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


bool AreEqual(const ValueContainer& a, const ValueContainer& b)
{
	if (a.type == b.type && a.type == ValueType::BOOL)
	{
		return a.as.boolean == b.as.boolean;
	}
	else if (a.type == b.type && a.type == ValueType::FLOAT)
	{
		return fabs(a.as.numberFloat - b.as.numberFloat) < 0.00004;
	}
	return false;
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
			vmStack.push(ValueContainer{ constants[opCode[++ipIndex]] });
			++ipIndex;
			break;
		}
		case InCode::TRUE:
		{
			vmStack.push(ValueContainer{ true });
			++ipIndex;
			break;
		}
		case InCode::FALSE:
		{
			vmStack.push(ValueContainer{ false });
			++ipIndex;
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
			++ipIndex;
			break;
		}
		case InCode::NOT:
		{
			auto value = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{ !value.as.boolean });
			++ipIndex;
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
			auto  v2 = vmStack.top();
			vmStack.pop();
			auto  v1 = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{ AreEqual(v1,v2) });
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

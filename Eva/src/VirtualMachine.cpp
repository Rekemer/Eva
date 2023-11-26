#include "VirtualMachine.h"
#include "AST.h"
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
	ADD_FLOAT,
	ADD_INT,
	DIVIDE_INT,
	DIVIDE_FLOAT,
	MULTIPLY_INT,
	MULTIPLY_FLOAT,
	SUBSTRACT_FLOAT,
	SUBSTRACT_INT,
	NEGATE,
	GREATER,
	LESS,
	EQUAL_EQUAL,
	AND,
	OR,
	NOT,
	PRINT,
	RETURN,

};

#define DETERMINE_NUMBER(left,right,OP)\
{\
	if (left == ValueType::INT && right == ValueType::INT)\
	{\
	opCode.push_back((uint8_t)InCode::OP##_INT); \
	return ValueType::INT; \
	}\
	else\
	{\
	opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	return ValueType::FLOAT; \
	}\
}\

ValueType VirtualMachine::Generate(const Expression * tree)
{
		if (!tree) return ValueType::NIL;

		if (tree->type == TokenType::PLUS)
		{
			auto left = Generate(tree->left);
			auto right = Generate(tree->right);
			DETERMINE_NUMBER(left, right, ADD);
		}
		else if (tree->type == TokenType::STAR)
		{
			auto left = Generate(tree->left);
			auto right = Generate(tree->right);
			DETERMINE_NUMBER(left, right, MULTIPLY);
		}
		else if (tree->type == TokenType::MINUS)
		{
			auto left = Generate(tree->left);

			if (tree->right)
			{
				auto right = Generate(tree->right);
				DETERMINE_NUMBER(left, right, SUBSTRACT);
			}
			else
			{
				opCode.push_back((uint8_t)InCode::NEGATE);
				return left;
			}

		}
		else if (tree->type == TokenType::SLASH)
		{
			auto left = Generate(tree->left);
			auto right = Generate(tree->right);
			DETERMINE_NUMBER(left, right, DIVIDE);
		}
		else if (tree->type == TokenType::INT_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.push_back(ValueContainer{ tree->value.as.numberInt });
			opCode.push_back(constants.size() - 1);
			return ValueType::INT;
		}
		else if (tree->type == TokenType::FLOAT_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.push_back(ValueContainer{ tree->value.as.numberFloat });
			opCode.push_back(constants.size() - 1);
			return ValueType::FLOAT;
		}
		else if (tree->type == TokenType::STRING_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			// might copy because vector can reallocate
			constants.emplace_back(tree->value.as.object );
			opCode.push_back(constants.size() - 1); 
			return ValueType::OBJ;
		}

		else if (tree->type == TokenType::TRUE)
		{
			opCode.push_back((uint8_t)InCode::TRUE);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::FALSE)
		{
			opCode.push_back((uint8_t)InCode::FALSE);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::GREATER)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::GREATER);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::GREATER_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::LESS);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::EQUAL_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::AND)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::AND);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::OR)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::OR);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::LESS_EQUAL)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::GREATER);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::LESS)
		{
			Generate(tree->left);
			Generate(tree->right);
			opCode.push_back((uint8_t)InCode::LESS);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::BANG)
		{
			Generate(tree->left);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::PRINT)
		{
			Generate(tree->left);
			opCode.push_back((uint8_t)InCode::PRINT);
			return ValueType::NIL;
		}
		

}

Object* VirtualMachine::AllocateString(const char* ptr, size_t size)
{
	if (internalStrings.IsExist(ptr))
	{
		auto str = internalStrings.Get(ptr);
		return static_cast<Object*>(str->key);
	}
	auto* entry = internalStrings.Add(std::string_view{ptr,size}, ValueContainer{});
	return static_cast<Object*>(entry->key);
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
	else if (a.type == b.type && a.type == ValueType::INT)
	{
		return a.as.numberInt == b.as.numberInt;
	}
	else if (a.type == b.type && a.type == ValueType::OBJ)
	{
		auto str = static_cast<String*>(a.as.object);
		auto str2 = static_cast<String*>(b.as.object);
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
		case InCode::ADD_FLOAT:
		{
			BINARY_OP(numberFloat, +);
			break;
		}
		case InCode::SUBSTRACT_FLOAT:
		{
			BINARY_OP(numberFloat, -);
			break;
		}
		case InCode::MULTIPLY_FLOAT:
		{
			BINARY_OP(numberFloat,*);
			break;
		}
		case InCode::DIVIDE_FLOAT:
		{
			BINARY_OP(numberFloat ,/ );
			break;
		}
		case InCode::ADD_INT:
		{
			BINARY_OP(numberInt, +);
			break;
		}
		case InCode::SUBSTRACT_INT:
		{
			BINARY_OP(numberInt, -);
			break;
		}
		case InCode::MULTIPLY_INT:
		{
			BINARY_OP(numberInt, *);
			break;
		}
		case InCode::DIVIDE_INT:
		{
			BINARY_OP(numberInt, / );
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
		case InCode::AND:
		{
			BINARY_OP(boolean, && );
			break;
		}
		case InCode::OR:
		{
			BINARY_OP(boolean, ||);
			break;
		}
		case InCode::RETURN:
		{
			return;
		}
		case InCode::PRINT:
		{	auto& v = vmStack.top();
			std::cout << v << "\n";
			break;
		}
		default:
			break;
		}
	}
}

void VirtualMachine::GenerateBytecode(const std::vector<AST>& trees)
{
	for (auto& tree : trees)
	{
		Generate(tree.GetTree());
	}
	opCode.push_back((uint8_t)InCode::RETURN);
}

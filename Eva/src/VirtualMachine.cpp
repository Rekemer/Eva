#include "VirtualMachine.h"
#include "Incode.h"
#include "AST.h"
#include "Debug.h"
#include "String.hpp"
#include <cassert>
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.top().as.type;\
vmStack.pop();\
auto v2 = vmStack.top().as.type;\
vmStack.pop();\
vmStack.push(ValueContainer{v2 operation v});\
}\

#define UNARY_OP(type,operation)\
{\
auto v = vmStack.top().as.type;\
vmStack.pop();\
auto v2 = vmStack.top().as.type;\
vmStack.pop();\
vmStack.push(ValueContainer{v2 operation v});\
}\
while(false)


#define DETERMINE_NUMBER_RET(type,OP)\
{\
	if (type == ValueType::INT)\
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

#define DETERMINE_NUMBER(type,OP)\
{\
	if (type == ValueType::INT)\
	{\
	opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else\
	{\
	opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
}\

#define DETERMINE_BOOL(left,right,OP)\
{\
	if (left == ValueType::INT && right == ValueType::INT)\
	{\
	opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else\
	{\
	opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
}\

#define CAST_INT_FLOAT(child,parentType)\
if (child== ValueType::INT && parentType== ValueType::FLOAT)\
{\
	opCode.push_back(((uint8_t)InCode::CAST_FLOAT));\
}\
if (child== ValueType::FLOAT && parentType== ValueType::INT)\
{\
	opCode.push_back(((uint8_t)InCode::CAST_INT));\
}\

ValueType VirtualMachine::Generate(const Expression * tree)
{
		if (!tree) return ValueType::NIL;
		if (tree->type == TokenType::BLOCK)
		{
			auto block = (Scope*)tree;
			for (auto expression : block->expressions)
			{
				Generate(expression);
			}
		}
		if (tree->type == TokenType::PLUS)
		{
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);

			DETERMINE_NUMBER_RET(tree->value.type ,ADD);
		}
		else if (tree->type == TokenType::PLUS_PLUS)
		{
			auto left = Generate(tree->left);



			DETERMINE_NUMBER(tree->value.type, INCREMENT);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->value.type;

		}
		else if (tree->type == TokenType::MINUS_MINUS)
		{
			auto left = Generate(tree->left);



			DETERMINE_NUMBER(tree->value.type, DECREMENT);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->value.type;

		}
		else if (tree->type == TokenType::STAR)
		{
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_NUMBER_RET(tree->value.type, MULTIPLY);
		}
		else if (tree->type == TokenType::MINUS)
		{
			auto left = Generate(tree->left);

			if (tree->right)
			{	
				CAST_INT_FLOAT(left, tree->value.type);
				auto right = Generate(tree->right);
				CAST_INT_FLOAT(right, tree->value.type);
				DETERMINE_NUMBER_RET(tree->value.type, SUBSTRACT);
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
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_NUMBER_RET(tree->value.type ,DIVIDE);
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
			return ValueType::STRING;
		}
		else if (tree->type == TokenType::IDENTIFIER)
		{
			//if (tree->left == nullptr)
			//{
				opCode.push_back((uint8_t)InCode::GET_VAR);
				constants.emplace_back(tree->value.as.object);
				opCode.push_back(constants.size() - 1);
				auto str = (String*)tree->value.as.object;
				auto entry = globalVariablesTypes.Get(str->GetStringView());
				return entry->value.type;
			//}


			
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
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_BOOL(left,right, GREATER);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::GREATER_EQUAL)
		{
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_BOOL(left, right, LESS);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::EQUAL_EQUAL)
		{
			auto left = Generate(tree->left);
			auto right = Generate(tree->right);
			opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::EQUAL)
		{
			auto expressionType = Generate(tree->right);
			
			assert(tree->left!= nullptr);
			auto str = (String*)tree->left->value.as.object;
			auto entry = globalVariablesTypes.Get(str->GetStringView());

			assert(entry->key != nullptr);

			CAST_INT_FLOAT(expressionType, entry->value.type);
			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->left->value.type;

		}
		else if (tree->type == TokenType::PLUS_EQUAL)
		{
			auto left = Generate(tree->left);
			auto expressionType = Generate(tree->right);

			assert(tree->left != nullptr);
			auto str = (String*)tree->left->value.as.object;
			auto entry = globalVariablesTypes.Get(str->GetStringView());

			assert(entry->key != nullptr);

			CAST_INT_FLOAT(expressionType, entry->value.type);


			DETERMINE_NUMBER(tree->left->value.type, ADD);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->left->value.type;
		}
		else if (tree->type == TokenType::STAR_EQUAL)
		{
			auto left = Generate(tree->left);
			auto expressionType = Generate(tree->right);

			assert(tree->left != nullptr);
			auto str = (String*)tree->left->value.as.object;
			auto entry = globalVariablesTypes.Get(str->GetStringView());

			assert(entry->key != nullptr);

			CAST_INT_FLOAT(expressionType, entry->value.type);


			DETERMINE_NUMBER(tree->left->value.type, MULTIPLY);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->left->value.type;
			}
		else if (tree->type == TokenType::SLASH_EQUAL)
		{
			auto left = Generate(tree->left);
			auto expressionType = Generate(tree->right);

			assert(tree->left != nullptr);
			auto str = (String*)tree->left->value.as.object;
			auto entry = globalVariablesTypes.Get(str->GetStringView());

			assert(entry->key != nullptr);

			CAST_INT_FLOAT(expressionType, entry->value.type);


			DETERMINE_NUMBER(tree->left->value.type, DIVIDE);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->left->value.type;
		}
		else if (tree->type == TokenType::MINUS_EQUAL)
		{
			auto left = Generate(tree->left);
			auto expressionType = Generate(tree->right);

			assert(tree->left != nullptr);
			auto str = (String*)tree->left->value.as.object;
			auto entry = globalVariablesTypes.Get(str->GetStringView());

			assert(entry->key != nullptr);

			CAST_INT_FLOAT(expressionType, entry->value.type);


			DETERMINE_NUMBER(tree->left->value.type, SUBSTRACT);


			constants.emplace_back(tree->left->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_VAR);
			opCode.push_back(constants.size() - 1);
			return tree->left->value.type;
			}
		else if (tree->type == TokenType::LESS_EQUAL)
		{
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_BOOL(left, right, GREATER);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::LESS)
		{
			auto left = Generate(tree->left);
			CAST_INT_FLOAT(left, tree->value.type);
			auto right = Generate(tree->right);
			CAST_INT_FLOAT(right, tree->value.type);
			DETERMINE_BOOL(left, right, LESS);
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
		else if (tree->type == TokenType::IF)
		{
			Generate(tree->left);
			opCode.push_back((uint8_t)InCode::JUMP_IF_FALSE);
			// the address of else byteblock code - backpatching
			// then branch
			opCode.push_back((uint8_t)0);
			auto indexJumpFalse = opCode.size() - 1;

			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->right->right);
			opCode.push_back((uint8_t)InCode::JUMP);
			opCode.push_back((uint8_t)0);
			auto indexJump= opCode.size() -1;
			opCode[indexJumpFalse] = indexJump - indexJumpFalse;
			// else branch
			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->right->left);
			// once we execute then branch we need to skip else bytecode
			// without -1 because we need next bytecode
			opCode[indexJump] = opCode.size()   - indexJump;

		}

}

Object* VirtualMachine::AllocateString(const char* ptr, size_t size)
{
	if (internalStrings.IsExist(std::string_view{ ptr,size }))
	{
		auto str = internalStrings.Get(std::string_view{ ptr,size });
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
	else if (a.type == b.type && a.type == ValueType::STRING)
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


	#if DEBUG
	Debug(opCode, constants);
	#endif

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
		case InCode::CAST_FLOAT:
		{
			auto& value = vmStack.top();
			value.type = ValueType::FLOAT;
			value.as.numberFloat = value.as.numberInt;
			break;
		}
		case InCode::CAST_INT:
		{
			auto& value = vmStack.top();
			value.type = ValueType::INT;
			value.as.numberInt= value.as.numberFloat;
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
		case InCode::INCREMENT_INT:
		{
			auto& value = vmStack.top();
			value.AsRef<int>()++;
			break;
		}
		case InCode::DECREMENT_INT:
		{
			auto& value = vmStack.top();
			value.AsRef<int>()--;
			break;
		}
		case InCode::INCREMENT_FLOAT:
		{
			auto& value = vmStack.top();
			value.AsRef<float>()++;
			break;
		}
		case InCode::DECREMENT_FLOAT:
		{
			auto& value = vmStack.top();
			value.AsRef<float>()--;
			break;
		}
		case InCode::NEGATE:
		{
			auto value = vmStack.top();
			vmStack.pop();
			if (value.type == ValueType::FLOAT)
			{
				vmStack.push(ValueContainer{-value.as.numberFloat});
			}
			else if (value.type == ValueType::INT)
			{
				vmStack.push(ValueContainer{-value.as.numberInt});
			}
			else
			{
				assert("Unknown type to negate" && false);
			}
			break;
		}
		case InCode::NOT:
		{
			auto value = vmStack.top();
			vmStack.pop();
			vmStack.push(ValueContainer{ !value.as.boolean });
			break;
		}
		case InCode::LESS_FLOAT:
		{
			BINARY_OP(numberFloat, < );
			break;
		}
		case InCode::GREATER_FLOAT:
		{
			BINARY_OP(numberFloat, > );
			break;
		}
		case InCode::LESS_INT:
		{
			BINARY_OP(numberInt, < );
			break;
		}
		case InCode::GREATER_INT:
		{
			BINARY_OP(numberInt, > );
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
		case InCode::GET_VAR:
		{	
			auto& nameOfVariable = constants[opCode[ipIndex++]];
			auto string = ((String*)(nameOfVariable.As<Object*>()))->GetStringView();
			auto entry = globalVariables.Get(string);
			vmStack.push(entry->value);
			break;
		}
		case InCode::SET_VAR:
		{	
			auto& value = vmStack.top();
			auto& nameOfVariable = constants[opCode[ipIndex++]];
			auto string = ((String*)(nameOfVariable.As<Object*>()))->GetStringView();
			auto entry = globalVariables.Get(string);
			entry->value = value;
			//vmStack.pop();
			break;
		}
		case InCode::JUMP_IF_FALSE:
		{
			// if it is not false, then we should get to then block
			auto offset = opCode[ipIndex++];
			auto condition = vmStack.top().As<bool>();
			if (!condition) ipIndex += offset;
			break;
		}
		case InCode::JUMP:
		{
			auto offset = opCode[ipIndex];
			ipIndex += offset;
			break;
		}
		case InCode::POP:
		{
			vmStack.pop();
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

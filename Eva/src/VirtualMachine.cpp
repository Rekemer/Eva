#include "VirtualMachine.h"
#include "Incode.h"
#include "AST.h"
#include "Debug.h"
#include "String.hpp"
#include <cassert>
#include <algorithm>
#include <sstream>
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.back().as.type;\
vmStack.pop_back();\
auto v2 = vmStack.back().as.type;\
vmStack.pop_back();\
vmStack.push_back(ValueContainer{v2 operation v});\
}\

#define UNARY_OP(type,operation)\
{\
auto v = vmStack.back().as.type;\
vmStack.pop_back();\
auto v2 = vmStack.back().as.type;\
vmStack.pop_back();\
vmStack.push_back(ValueContainer{v2 operation v});\
}\
while(false)


#define DETERMINE_OP_TYPE_RET(type,OP)\
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
// deterni
#define DETERMINE_OP_TYPE(type,OP)\
{\
	if (type == ValueType::INT)\
	{\
	opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else if (type == ValueType::FLOAT)\
	{\
	opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
	else{assert(false && "unknown type of operation");}\
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
else if (child== ValueType::FLOAT && parentType== ValueType::INT)\
{\
	opCode.push_back(((uint8_t)InCode::CAST_INT));\
}\
else\
{\
  /*do nothing*/\
}

// returns the index for backpatching
int JumpIfFalse(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_IF_FALSE);
	// the address of else byteblock code - backpatching
	// then branch
	opCode.push_back((uint8_t)0);
	auto indexJumpFalse = opCode.size() - 1;
	return indexJumpFalse;
}
int Jump(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
int JumpBack(std::vector<Bytecode>& opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_BACK);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
int CalculateJumpIndex(std::vector<Bytecode>& opCode, int from)
{
	return opCode.size() - 1 - from;
}

ValueType VirtualMachine::GetVariable(std::vector<Bytecode>& opCode, const Expression* expr)
{
	// global
	if (expr->depth == 0)
	{
		opCode.push_back((uint8_t)InCode::GET_GLOBAL_VAR);
		constants.emplace_back(expr->value.as.object);
		opCode.push_back(constants.size() - 1);
		auto str = (String*)expr->value.as.object;
		auto entry = globalVariablesTypes.Get(str->GetStringView());
		return entry->value.type;
	}
	// local
	else if (expr->depth > 0)
	{
		auto str = (String*)expr->value.as.object;
		// check if it does exsist
		// maybe we should move it to ast tree, but the indexing will get 
		// compilcated since we don't know in indexing how much scopes we have passed
		auto [isDeclared,index] = IsLocalExist(*str);
	
		opCode.push_back((uint8_t)InCode::GET_LOCAL_VAR);
		opCode.push_back(index);

		Entry* entry = nullptr;
		for (auto& scope : currentScopes)
		{
			entry = scope->types.Get(str->GetStringView());
			if (entry->key != nullptr) break;
		}
		if (entry->key == nullptr)
		{
			std::stringstream ss;
			ss << "ERROR[" << (expr->line) << "]: " <<
				"The name " << str->GetRaw() << " is used but not declared " << std::endl;
			throw std::exception{ss.str().c_str()};
		}
		return entry->value.type;
	}
}




void VirtualMachine::SetVariable(std::vector<Bytecode>& opCode,const Expression* expression)
{
	assert(expression != nullptr);
	auto str = (String*)expression->value.as.object;
	if (expression->depth == 0)
	{
		constants.emplace_back(expression->value.as.object);
		opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
		opCode.push_back(constants.size() - 1);
	}
	else
	{
		auto [isDeclared, index] = IsLocalExist(*str);
		opCode.push_back((uint8_t)InCode::SET_LOCAL_VAR);
		opCode.push_back(index);
	}
}

ValueType VirtualMachine::Generate(const Node * tree)
{
		if (!tree) return ValueType::NIL;
		 auto expr = static_cast<const Expression*>(tree);
		 auto exprLeft = static_cast<const Expression*>(tree->As<Expression>()->left.get());
		if (tree->type == TokenType::BLOCK)
		{
			auto block = static_cast<const Scope*>(tree);
			// empty block
			if (block->expressions.size() == 0)
			{
				return{};
			}
			currentScopes.push_back(block);
			for (auto& expression : block->expressions)
			{
				Generate(expression.get());
			}
			// once we finish block, we must clear the stack
			int popAmount = block->popAmount;
			while (popAmount > 0 )
			{
				opCode.push_back((uint8_t)InCode::POP);
				popAmount--;
			}
			currentScopes.pop_back();
			localPtr -= block->popAmount;
			assert(localPtr >= 0);

		}
		else if (tree->type == TokenType::PLUS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);

			DETERMINE_OP_TYPE_RET(expr->value.type ,ADD);
		}
		else if (tree->type == TokenType::PLUS_PLUS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());



			DETERMINE_OP_TYPE(left, INCREMENT);


			constants.emplace_back(exprLeft->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
			opCode.push_back(constants.size() - 1);
			return expr->value.type;

		}
		else if (tree->type == TokenType::MINUS_MINUS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());



			DETERMINE_OP_TYPE(left, DECREMENT);


			constants.emplace_back(exprLeft->value.as.object);
			opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
			opCode.push_back(constants.size() - 1);
			return expr->value.type;

		}
		else if (tree->type == TokenType::STAR)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_OP_TYPE_RET(expr->value.type, MULTIPLY);
		}
		else if (tree->type == TokenType::MINUS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());

			if (tree->As<Expression>()->right.get())
			{	
				CAST_INT_FLOAT(left, expr->value.type);
				auto right = Generate(tree->As<Expression>()->right.get());
				CAST_INT_FLOAT(right, expr->value.type);
				DETERMINE_OP_TYPE_RET(expr->value.type, SUBSTRACT);
			}
			else
			{
				opCode.push_back((uint8_t)InCode::NEGATE);
				return left;
			}

		}
		else if (tree->type == TokenType::SLASH)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_OP_TYPE_RET(expr->value.type ,DIVIDE);
		}
		else if (tree->type == TokenType::INT_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.push_back(ValueContainer{ expr->value.as.numberInt });
			opCode.push_back(constants.size() - 1);
			return ValueType::INT;
		}
		else if (tree->type == TokenType::FLOAT_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			constants.push_back(ValueContainer{ expr->value.as.numberFloat });
			opCode.push_back(constants.size() - 1);
			return ValueType::FLOAT;
		}
		else if (tree->type == TokenType::STRING_LITERAL)
		{
			opCode.push_back((uint8_t)InCode::CONST_VALUE);
			// might copy because vector can reallocate
			constants.emplace_back(expr->value.as.object );
			opCode.push_back(constants.size() - 1); 
			return ValueType::STRING;
		}
		else if (tree->type == TokenType::IDENTIFIER)
		{
			return GetVariable(opCode, expr);
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
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_BOOL(left,right, GREATER);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::GREATER_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_BOOL(left, right, LESS);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::EQUAL_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto right = Generate(tree->As<Expression>()->right.get());
			opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::BANG_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto right = Generate(tree->As<Expression>()->right.get());
			opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
			}
		else if (tree->type == TokenType::EQUAL)
		{
			// declaring a variable
			auto expressionType = Generate(tree->As<Expression>()->right.get());
			assert(tree->As<Expression>()->left.get()!= nullptr);
			auto str = (String*)exprLeft->value.as.object;

			CAST_INT_FLOAT(expressionType, exprLeft->value.type);
			SetVariable(opCode, exprLeft);

		}
		else if (tree->type == TokenType::PLUS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());

			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, ADD);

			SetVariable(opCode, exprLeft);
			return exprLeft->value.type;
		}
		else if (tree->type == TokenType::STAR_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, MULTIPLY);

			SetVariable(opCode, exprLeft);

			return exprLeft->value.type;
			}
		else if (tree->type == TokenType::SLASH_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, DIVIDE);


			SetVariable(opCode, exprLeft);
			return exprLeft->value.type;
		}
		else if (tree->type == TokenType::MINUS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, SUBSTRACT);


			SetVariable(opCode, exprLeft);
			return exprLeft->value.type;
			}
		else if (tree->type == TokenType::LESS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_BOOL(left, right, GREATER);
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::LESS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_BOOL(left, right, LESS);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::AND)
		{
			Generate(tree->As<Expression>()->left.get());
			// check if it is false then we just ignore second operand
			// and leave the value on stack
			auto indexFalse = JumpIfFalse(opCode);
			// remove the value because we didn't jump
			// the whole and is dependent on second operand
			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get());
			//opCode.push_back((uint8_t)InCode::AND);
			auto jumpLen = CalculateJumpIndex(opCode, indexFalse);
			opCode[indexFalse] = jumpLen;
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::OR)
		{
			Generate(tree->As<Expression>()->left.get());
			auto indexFalse = JumpIfFalse(opCode);
			// if it is true we get to jump
			auto jump = Jump(opCode);
			
			auto jumpLen = CalculateJumpIndex(opCode,indexFalse);
			opCode[indexFalse] = jumpLen;

			Generate(tree->As<Expression>()->right.get());
			// the first is true- just skip second operand
			jumpLen = CalculateJumpIndex(opCode, jump);
			opCode[jump] = jumpLen;
			opCode.push_back((uint8_t)InCode::OR);
			return ValueType::BOOL;
		}
		
		else if (tree->type == TokenType::BANG)
		{
			Generate(tree->As<Expression>()->left.get());
			opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::PRINT)
		{
			Generate(tree->As<Expression>()->left.get());
			opCode.push_back((uint8_t)InCode::PRINT);
			return ValueType::NIL;
		}
		else if (tree->type == TokenType::IF)
		{
			Generate(tree->As<Expression>()->left.get());
			auto indexJumpFalse = JumpIfFalse(opCode);

			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get()->As<Expression>()->right.get());
			auto indexJump = Jump(opCode);
			opCode[indexJumpFalse] = indexJump - indexJumpFalse;
			// else branch
			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get()->As<Expression>()->left.get());
			// once we execute then branch we need to skip else bytecode
			// without -1 because we need index of next bytecode, not previous one
			opCode[indexJump] = opCode.size()   - indexJump;

		}
		else if (tree->type == TokenType::WHILE)
		{
			auto startIndex = opCode.size();
			Generate(tree->As<Expression>()->left.get());
			auto indexJumpFalse = JumpIfFalse(opCode);
			opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get());
			auto jump = JumpBack(opCode);
			auto len = CalculateJumpIndex(opCode, indexJumpFalse);
			opCode[indexJumpFalse] = len;
			// jumping backwards
			auto negativeOffset= (CalculateJumpIndex(opCode, startIndex));
			opCode[jump] = negativeOffset;

		}
		else if (tree->type== TokenType::FOR)
		{
			auto forNode = tree->As<For>();
			Generate(forNode->init.get());
			auto startIndex = opCode.size();
			Generate(forNode->condition.get());
			auto indexJumpFalse = JumpIfFalse(opCode);
			opCode.push_back((uint8_t)InCode::POP);
			Generate(forNode->action.get());
			Generate(forNode->body.get());
			auto jump = JumpBack(opCode);
			opCode[indexJumpFalse] = CalculateJumpIndex(opCode, indexJumpFalse);
			opCode[jump] = CalculateJumpIndex(opCode, startIndex);
		}
		else
		{
			assert(false && "weird type");
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

void VirtualMachine::AddLocal(String& name, int currentScope)
{
	auto iter = std::find_if(locals.begin(), locals.end(), [&](auto& local)
		{
			return local.name == name;
		});
	if (iter != locals.end()) assert(false && "variable already declared");
	locals[localPtr].name = name;
	locals[localPtr++].depth = currentScope;
}

std::tuple<bool, int> VirtualMachine::IsLocalExist(String& name)
{
	auto temp = localPtr;
	while (temp >= 0 )
	{
		if (name == locals[temp].name)
		{
			return { true ,temp };
		}
		temp--;
	}
	return { false ,-1 };
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
	if (m_Panic)return;

	#if DEBUG
	Debug(*this);
	#endif

	while (true)
	{
		auto inst = opCode[ipIndex++];

		switch (static_cast<InCode>(inst))
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push_back(constants[opCode[ipIndex++]]);
			break;
		}
		case InCode::TRUE:
		{
			vmStack.push_back(ValueContainer{ true });
			break;
		}
		case InCode::FALSE:
		{
			vmStack.push_back(ValueContainer{ false });
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
			auto& value = vmStack.back();
			value.type = ValueType::FLOAT;
			value.as.numberFloat = value.as.numberInt;
			break;
		}
		case InCode::CAST_INT:
		{
			auto& value = vmStack.back();
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
			auto& value = vmStack.back();
			value.AsRef<int>()++;
			break;
		}
		case InCode::DECREMENT_INT:
		{
			auto& value = vmStack.back();
			value.AsRef<int>()--;
			break;
		}
		case InCode::INCREMENT_FLOAT:
		{
			auto& value = vmStack.back();
			value.AsRef<float>()++;
			break;
		}
		case InCode::DECREMENT_FLOAT:
		{
			auto& value = vmStack.back();
			value.AsRef<float>()--;
			break;
		}
		case InCode::NEGATE:
		{
			auto value = vmStack.back();
			vmStack.pop_back();
			if (value.type == ValueType::FLOAT)
			{
				vmStack.push_back(ValueContainer{-value.as.numberFloat});
			}
			else if (value.type == ValueType::INT)
			{
				vmStack.push_back(ValueContainer{-value.as.numberInt});
			}
			else
			{
				assert("Unknown type to negate" && false);
			}
			break;
		}
		case InCode::NOT:
		{
			auto value = vmStack.back();
			vmStack.pop_back();
			vmStack.push_back(ValueContainer{ !value.as.boolean });
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
			auto&  v2 = vmStack.back();
			vmStack.pop_back();
			auto&  v1 = vmStack.back();
			vmStack.pop_back();
			vmStack.push_back(ValueContainer{ AreEqual(v1,v2) });
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
		{	auto& v = vmStack.back();
			std::cout << v << "\n";
			vmStack.pop_back();
			break;
		}
		case InCode::GET_GLOBAL_VAR:
		{	
			auto& nameOfVariable = constants[opCode[ipIndex++]];
			auto string = static_cast<String*>(nameOfVariable.As<Object*>())->GetStringView();
			auto entry = globalVariables.Get(string);
			vmStack.push_back(entry->value);
			break;
		}
		case InCode::SET_GLOBAL_VAR:
		{	
			auto& value = vmStack.back();
			auto& nameOfVariable = constants[opCode[ipIndex++]];
			auto string = ((String*)(nameOfVariable.As<Object*>()))->GetStringView();
			auto entry = globalVariables.Get(string);
			entry->value = value;
			vmStack.pop_back();
			break;
		}

		case InCode::GET_LOCAL_VAR:
		{
			auto index = opCode[ipIndex++];
			vmStack.push_back(vmStack[index]);
			break;
		}
		case InCode::SET_LOCAL_VAR:
		{
			auto index = opCode[ipIndex++];
			auto value = vmStack.back();
			vmStack[index] = value;
			break;
		}

		case InCode::JUMP_IF_FALSE:
		{
			// if it is not false, then we should get to then block
			auto offset = opCode[ipIndex++];
			auto condition = vmStack.back().As<bool>();
			if (!condition) ipIndex += offset;
			break;
		}
		case InCode::JUMP:
		{
			auto offset = opCode[ipIndex];
			ipIndex += offset;
			break;
		}
		case InCode::JUMP_BACK:
		{
			auto offset = opCode[ipIndex];
			ipIndex -= offset;
			break;
		}
		case InCode::POP:
		{
			vmStack.pop_back();
		}
		default:
			break;
		}
	}
}

void VirtualMachine::GenerateBytecode(const std::vector<AST>& trees)
{
	try
	{
		for (auto& tree : trees)
		{
			Generate(tree.GetTree());
		}
		opCode.push_back((uint8_t)InCode::RETURN);
	}
	catch (const std::exception& e)
	{
		m_Panic = true;
		std::cout << e.what();
	}
	
}

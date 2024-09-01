#include "VirtualMachine.h"
#include "Incode.h"
#include "AST.h"
#include "Debug.h"
#include "String.hpp"
#include <cassert>
#include <algorithm>
#include <sstream>
#include"TokenConversion.h"
#define BINARY_OP(type,operation)\
{\
auto v = vmStack.back().As<type>();\
vmStack.pop_back();\
auto v2 = vmStack.back().As<type>();\
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
	currentFunc->opCode.push_back((Bytecode)InCode::OP##_INT); \
	return ValueType::INT; \
	}\
	else if (type == ValueType::FLOAT)\
	{\
	currentFunc->opCode.push_back((Bytecode)InCode::OP##_FLOAT); \
	return ValueType::FLOAT; \
	}\
	else\
	{\
	assert(false&& "type checking didn't work"); \
	return ValueType::NIL; \
	}\
}\
// deterni
#define DETERMINE_OP_TYPE(type,OP)\
{\
	if (type == ValueType::INT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else if (type == ValueType::FLOAT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
	else{assert(false && "unknown type of operation");}\
}\

#define DETERMINE_BOOL(left,right,OP)\
{\
	if (left == ValueType::INT && right == ValueType::INT)\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_INT); \
	}\
	else\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	}\
}\

#define CAST_BOOL(type)\
if (type== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_BOOL_INT));\
}else if (type== ValueType::FLOAT)\
{\
currentFunc->opCode.push_back(((uint8_t)InCode::CAST_BOOL_FLOAT));\
}\

#define CAST_INT_FLOAT(child,parentType)\
if (child== ValueType::INT && parentType== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_FLOAT));\
}\
else if (child== ValueType::FLOAT && parentType== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_INT));\
}\

#define CAST_INT(child)\
if (child== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((uint8_t)InCode::CAST_INT));\
}\

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
// from: to make jump relative
int CalculateJumpIndex(const std::vector<Bytecode> const &  opCode, const int from)
{
	return opCode.size() - 1 - from;
}

void ClearScope(std::vector<const Scope*>& scopes, int& stackPtr,
	std::vector<Bytecode>& opCode)
{
	int popAmount = scopes.back()->popAmount;
	while (popAmount > 0)
	{
		opCode.push_back((uint8_t)InCode::POP);
		popAmount--;
	}

	stackPtr -= scopes.back()->popAmount;
	scopes.pop_back();
	assert(stackPtr >= 0);
}

ValueType VirtualMachine::GetVariable(std::vector<Bytecode>& opCode, const Expression* expr)
{
	// global
	if (expr->depth == 0)
	{
		currentFunc->opCode.push_back((uint8_t)InCode::GET_GLOBAL_VAR);
		auto str = expr->value.AsString();
		currentFunc->constants.emplace_back(str);
		currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
		auto entry = globalVariablesTypes.Get(str->GetStringView());
		return entry->value.type;
	}
	// local
	else if (expr->depth > 0)
	{
		auto str = expr->value.AsString();
		// check if it does exsist
		// maybe we should move it to ast tree, but the indexing will get 
		// compilcated since we don't know in indexing how much scopes we have passed
		auto [isDeclared,index] = IsLocalExist(*str, expr->depth);
	
		currentFunc->opCode.push_back((uint8_t)InCode::GET_LOCAL_VAR);
		currentFunc->opCode.push_back(index);

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
	auto str = expression->value.AsString();;
	if (expression->depth == 0)
	{
		currentFunc->constants.emplace_back(str);
		currentFunc->opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
		currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
	}
	else
	{
		auto [isDeclared, index] = IsLocalExist(*str, expression->depth);
		currentFunc->opCode.push_back((uint8_t)InCode::SET_LOCAL_VAR);
		currentFunc->opCode.push_back(index);
	}
}
int VirtualMachine::GenerateLoopCondition(const Node* node)
{
	Generate(node);
	auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);
	currentFunc->opCode.push_back((uint8_t)InCode::POP);
	return indexJumpFalse;
}

void VirtualMachine::BeginContinue(int startLoopIndex)
{
	m_StartLoopIndexes.push(startLoopIndex);
}
void VirtualMachine::EndContinue()
{
	m_StartLoopIndexes.pop();
}
int VirtualMachine::BeginBreak()
{
	return m_BreakIndexes.size();
}
void VirtualMachine::PatchBreak(int prevSizeBreak)
{
	// we hit break we should patch it
	if (m_BreakIndexes.size() - prevSizeBreak > 0)
	{
		// we need to skip one pop because there is condition check on the stack 
		// when we execute body of the loop, hence +2. 
		// 1 to get to the future instruction
		// 1 to skip pop opeation
		auto index = m_BreakIndexes.top();
		currentFunc->opCode[index] =
			CalculateJumpIndex(currentFunc->opCode, index) + 2;
		m_BreakIndexes.pop();
	}
}
ValueType VirtualMachine::GetVariableType(const String* name, int depthOfDeclaration)
{
	if (depthOfDeclaration > 0)
	{
		Entry* entry = nullptr;
		entry = currentScopes[depthOfDeclaration - 1]->types.Get(name->GetStringView());
		if (entry->key != nullptr) return entry->value.type;
		assert(false);
	}
	else
	{
		return globalVariablesTypes.Get(name->GetStringView())->value.type;
	}
}
ValueType VirtualMachine::Generate(const Node * tree)
{
		 if (!tree) return ValueType::NIL;
		 auto expr = static_cast<const Expression*>(tree);
		 auto exprLeft = static_cast<const Expression*>(tree->As<Expression>()->left.get());


		 switch (tree->type)
		 {
		 case TokenType::FUN:
		 {
			 auto func = static_cast<const FunctionNode*>(tree);
			 functionNames.push_back(func->name);
			 auto funcValue = globalVariables.Get(func->name->GetStringView())->
				 value.AsFunc();

			 funcValue->name = func->name;
			 if (*func->name == "main")
			 {
				 mainFunc = funcValue.get();
			 }
			 currentFunc = funcValue.get();
			 currentScopes.push_back(&func->paramScope);
			 for (auto& arg : func->arguments)
			 {
				 Generate(arg.get());
			 }
			 Generate(func->body.get());
			 auto type = globalVariablesTypes.Get(func->name->GetStringView())->value.type;
			 ClearLocal();
			 currentScopes.pop_back();
			 if (type == ValueType::NIL)
			 {
				 currentFunc->opCode.push_back((uint8_t)InCode::NIL);
				 currentFunc->opCode.push_back((uint8_t)InCode::RETURN);
				 return ValueType::NIL;
			 }
			 else return type;
			 break;
			 //ClearScope(currentScopes,m_StackPtr, currentFunc->opCode);
		 }
		 case TokenType::RETURN:
		 {
			 auto value = Generate(exprLeft);
			 currentFunc->opCode.push_back((uint8_t)InCode::RETURN);
			 break;
		 }
		 case TokenType::LEFT_PAREN:
		 {
			 // invoke function

			 auto call = static_cast<const Call*>(tree);
			 currentFunc->opCode.push_back((uint8_t)InCode::GET_GLOBAL_VAR);
			 currentFunc->constants.emplace_back((call->name));
			 currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			 auto funcValue = globalVariables.Get(call->name->GetStringView())->value.AsFunc();
			 for (auto i = 0; i < call->args.size(); i++)
			 {
				 auto& arg = call->args[i];
				 auto argType = Generate(arg.get());
				 auto declType = funcValue->argTypes[i];
				 CAST_INT_FLOAT(argType,declType);

			 }
			 currentFunc->opCode.push_back((uint8_t)InCode::CALL);
			 currentFunc->opCode.push_back(call->args.size());
			 auto type = globalVariablesTypes.Get(call->name->GetStringView())->value.type;
			 if (type == ValueType::NIL)
			 {
				 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 }
			 break;
		 }
		 case TokenType::BLOCK:
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
			 ClearScope(currentScopes, m_StackPtr, currentFunc->opCode);
			 break;
		 }
		 case TokenType::PLUS:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 
			 if (left == right && left == ValueType::STRING)
			 {
				 currentFunc->opCode.push_back((Bytecode)InCode::ADD_STRING);
			 }
			 else
			 {
				DETERMINE_OP_TYPE_RET(expr->value.type, ADD);
			 }
			 break;
		 }
		 case TokenType::PLUS_PLUS:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 DETERMINE_OP_TYPE(left, INCREMENT);
			 SetVariable(currentFunc->opCode, exprLeft);
			 return expr->value.type;
			 break;
		 }
		 case TokenType::MINUS_MINUS:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 DETERMINE_OP_TYPE(left, DECREMENT);
			 SetVariable(currentFunc->opCode, exprLeft);
			 return expr->value.type;
			 break;
		 }
		 case TokenType::STAR:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_OP_TYPE_RET(expr->value.type, MULTIPLY);
			 break;
		 }
		 case TokenType::MINUS:
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
				 currentFunc->opCode.push_back((uint8_t)InCode::NEGATE);
				 return left;
			 }
			 break;
		 
		 }
		 case TokenType::SLASH:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_OP_TYPE_RET(expr->value.type, DIVIDE);
			 break;
		 }
		 case TokenType::PERCENT:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT(left);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT(right);
			 currentFunc->opCode.push_back((uint8_t)InCode::DIVIDE_PERCENT);
			 return ValueType::INT;
			 break;
		 }
		 case TokenType::INT_LITERAL:
		 {
			 currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			 currentFunc->constants.push_back(ValueContainer{ expr->value });
			 currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			 return ValueType::INT;
		 }
		 case TokenType::FLOAT_LITERAL:
		 {
			 currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			 currentFunc->constants.push_back(ValueContainer{ expr->value });
			 currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			 return ValueType::FLOAT;
		 }
		 case TokenType::STRING_LITERAL:
		 {
			 currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			 // might copy because vector can reallocate
			 currentFunc->constants.emplace_back(expr->value);
			 currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			 return ValueType::STRING;
		 }
		 case TokenType::IDENTIFIER:
		 {
			 auto type = GetVariable(currentFunc->opCode, expr);
			 return type;
		 }
		 case TokenType::TRUE:
		 {
			 currentFunc->opCode.push_back((uint8_t)InCode::TRUE);
			 return ValueType::BOOL;
			 }
		 case TokenType::FALSE:
		 {
			 currentFunc->opCode.push_back((uint8_t)InCode::FALSE);
			 return ValueType::BOOL;
		 }
		 case TokenType::GREATER:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, GREATER);
			 return ValueType::BOOL;
			 }
		 case TokenType::GREATER_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, LESS);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
		}
		 case TokenType::EQUAL_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto right = Generate(tree->As<Expression>()->right.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			 return ValueType::BOOL;
		}
		 case TokenType::BANG_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto right = Generate(tree->As<Expression>()->right.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
		}
		 case TokenType::DECLARE:
		 {
			 // declaring a variable
			 auto expressionType = Generate(tree->As<Expression>()->right.get());
			 assert(tree->As<Expression>()->left.get() != nullptr);
			 auto str = exprLeft->value.AsString();
			 auto declType = GetVariableType(str.get(), exprLeft->depth);
			 CAST_INT_FLOAT(expressionType, declType);


			 assert(exprLeft != nullptr);
			 if (exprLeft->depth == 0)
			 {
				 currentFunc->constants.emplace_back(exprLeft->value);
				 currentFunc->opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
				 currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			 }
			 break;
			}
		 case TokenType::EQUAL:
		 {
			 // declaring a variable
			 assert(tree->As<Expression>()->left.get() != nullptr);
			 auto declType = Generate(tree->As<Expression>()->left.get());
			 auto expressionType = Generate(tree->As<Expression>()->right.get());

			 CAST_INT_FLOAT(expressionType, declType);
			 SetVariable(currentFunc->opCode, exprLeft);
			 break;
			 }
		 case TokenType::PLUS_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto expressionType = Generate(tree->As<Expression>()->right.get());

			 CAST_INT_FLOAT(expressionType, left);


			 DETERMINE_OP_TYPE(left, ADD);

			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::STAR_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto expressionType = Generate(tree->As<Expression>()->right.get());


			 CAST_INT_FLOAT(expressionType, left);


			 DETERMINE_OP_TYPE(left, MULTIPLY);

			 SetVariable(currentFunc->opCode, exprLeft);

			 return exprLeft->value.type;
			 }
		 case TokenType::SLASH_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto expressionType = Generate(tree->As<Expression>()->right.get());


			 CAST_INT_FLOAT(expressionType, left);


			 DETERMINE_OP_TYPE(left, DIVIDE);


			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::MINUS_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 auto expressionType = Generate(tree->As<Expression>()->right.get());


			 CAST_INT_FLOAT(expressionType, left);


			 DETERMINE_OP_TYPE(left, SUBSTRACT);


			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::LESS_EQUAL:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, GREATER);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
			 }
		 case TokenType::LESS:
		 {
			 auto left = Generate(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = Generate(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, LESS);
			 return ValueType::BOOL;
			 }
		 case TokenType::AND:
		 {
			 Generate(tree->As<Expression>()->left.get());
			 // check if it is false then we just ignore second operand
			 // and leave the value on stack
			 auto indexFalse = JumpIfFalse(currentFunc->opCode);
			 // remove the value because we didn't jump
			 // the whole and is dependent on second operand
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 Generate(tree->As<Expression>()->right.get());
			 currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;
			 return ValueType::BOOL;
			 }
		 case TokenType::OR:
		 {
			 Generate(tree->As<Expression>()->left.get());
			 auto indexFalse = JumpIfFalse(currentFunc->opCode);
			 // if it is true we get to jump
			 auto jump = Jump(currentFunc->opCode);

			 currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;

			 Generate(tree->As<Expression>()->right.get());
			 // the first is true- just skip second operand
			 currentFunc->opCode.push_back((uint8_t)InCode::OR);
			 currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, jump) + 1;
			 return ValueType::BOOL;
			 }

		 case TokenType::BANG:
		 {
			 Generate(tree->As<Expression>()->left.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
			 }
		 case TokenType::PRINT:
		 {
			 Generate(tree->As<Expression>()->left.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::PRINT);
			 return ValueType::NIL;
			 }
		 case TokenType::IF:
		 {
			 Generate(tree->As<Expression>()->left.get());
			 auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);

			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 Generate(tree->As<Expression>()->right.get()->As<Expression>()->right.get());
			 auto indexJump = Jump(currentFunc->opCode);
			 currentFunc->opCode[indexJumpFalse] = (indexJump + 1) - indexJumpFalse;
			 // else branch
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 Generate(tree->As<Expression>()->right.get()->As<Expression>()->left.get());
			 // once we execute then branch we need to skip else bytecode
			 // without -1 because we need index of next bytecode, not previous one
			 currentFunc->opCode[indexJump] = currentFunc->opCode.size() - indexJump;

			 break;
			 }
		 case TokenType::WHILE:
		 {
			 auto startIndex = currentFunc->opCode.size();
			 auto condition = tree->As<Expression>()->left.get();
			 auto indexJumpFalse = GenerateLoopCondition(condition);

			 BeginContinue(startIndex);
			 auto prevSizeBreak = BeginBreak();

			 auto body = tree->As<Expression>()->right.get();
			 Generate(body);
			 auto jump = JumpBack(currentFunc->opCode);
			 // jumping backwards
			 currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, startIndex);

			 EndContinue();
			 PatchBreak(prevSizeBreak);

			 // clean the check condition 
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 currentFunc->opCode[indexJumpFalse] = CalculateJumpIndex(currentFunc->opCode, indexJumpFalse);
			 break;
			 }
		 case TokenType::FOR:
		 {
			 auto forNode = tree->As<For>();
			 currentScopes.push_back(&forNode->initScope);
			 Generate(forNode->init.get());
			 auto firstIteration = Jump(currentFunc->opCode);
			 auto startLoopIndex = currentFunc->opCode.size();

			 BeginContinue(startLoopIndex);
			 auto prevSizeBreak = BeginBreak();

			 Generate(forNode->action.get());
			 auto indexJumpFalse = GenerateLoopCondition(forNode->condition.get());
			 currentFunc->opCode[firstIteration] = CalculateJumpIndex(currentFunc->opCode, firstIteration) + 1;
			 Generate(forNode->body.get());
			 EndContinue();

			 auto jump = JumpBack(currentFunc->opCode);
			 currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, startLoopIndex);

			 PatchBreak(prevSizeBreak);


			 // clean the check condition once we go finish the loop
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 currentFunc->opCode[indexJumpFalse] = CalculateJumpIndex(currentFunc->opCode, indexJumpFalse);
			 // because for loop has declared iterator variable
			 ClearScope(currentScopes, m_StackPtr, currentFunc->opCode);
			 break;
		 }
		 case TokenType::CONTINUE:
		 {
			 int index = JumpBack(currentFunc->opCode);
			 assert(m_StartLoopIndexes.size() > 0);
			 currentFunc->opCode[index] = CalculateJumpIndex(currentFunc->opCode, m_StartLoopIndexes.top());
			 break;
		 }
		 case TokenType::BREAK:
		 {
			 m_BreakIndexes.push(Jump(currentFunc->opCode));
			 break;

		  }
		 default:
			 // should say what type it is
			 std::cout << "ERROR Code generation: weird type " << tokenToString(tree->type) << std::endl;
			 assert(false);
			 break;
		 }
		 
		

}


std::shared_ptr<String> VirtualMachine::AddStrings(std::shared_ptr<String> s, std::shared_ptr<String> s1) 
{
	auto raw1 = s->GetRaw();
	auto raw2 = s1->GetRaw();
	auto newSize = s->GetSize() + s1->GetSize();
	auto newString = new char[newSize + 1];
	strcpy(newString, raw1);
	strcat(newString, raw2);
	auto res = AllocateString(newString, newSize);
	// should move instead of  copy at the constructor of string
	delete[] newString;
	return res;

}

std::shared_ptr<String> VirtualMachine::AllocateString(const char* ptr, size_t size)
{
	if (internalStrings.IsExist(std::string_view{ ptr,size }))
	{
		auto str = internalStrings.Get(std::string_view{ ptr,size });
		return (str->key);
	}
	auto* entry = internalStrings.Add(std::string_view{ptr,size});
	return (entry->key);
}
std::string_view VirtualMachine::LastLocal()
{
	return locals[m_StackPtr - 1].name.GetStringView();
}
void VirtualMachine::AddLocal(String& name, int currentScope)
{
	auto endIterator = locals.begin() + m_StackPtr;
	auto iter = std::find_if(locals.begin(), endIterator, [&](auto& local)
		{
			return local.name == name && local.depth == currentScope;
		});

	if (iter != endIterator)
	{
		assert(false && "variable already declared in current scope");
	}
	locals[m_StackPtr].name = name;
	locals[m_StackPtr++].depth = currentScope;
}

std::tuple<bool, int> VirtualMachine::IsLocalExist(String& name, size_t scope)
{
	auto temp = m_StackPtr - 1;
	while (temp >= 0 )
	{
		if (name == locals[temp].name && scope >= locals[temp].depth)
		{
			return { true ,temp };
		}
		temp--;
	}
	return { false ,-1 };
}

void VirtualMachine::ClearLocal()
{
	m_StackPtr = 0;
}

VirtualMachine::~VirtualMachine()
{
}


bool VirtualMachine::AreEqual(const ValueContainer& a, const ValueContainer& b)
{
	if (a.type == b.type && a.type == ValueType::BOOL)
	{
		return a.As<bool>() == b.As<bool>();
	}
	else if (a.type == b.type && a.type == ValueType::FLOAT)
	{
		return fabs(a.As<float>() - b.As<float>()) < 0.04;
	}
	else if (a.type == b.type && a.type == ValueType::INT)
	{
		return a.As<int>() == b.As<int>();
	}
	else if (a.type == b.type && a.type == ValueType::STRING)
	{
		auto str = a.AsString();
		auto str2 = b.AsString();
		return *str == *str2;
	}
	return false;
}
void VirtualMachine::CollectStrings() 
{


	//auto a = internalStrings.Get(std::string_view{"Hello, New Year!", 16});
	//internalStrings.Print();
	
	//for (HashTable::Iterator i = internalStrings.begin(); i != internalStrings.end(); i++)
	//{
	//	std::cout << *i->key << std::endl;
	//}
	
	for (auto& entry : internalStrings)
	{
		if (entry.key.use_count() == 1)
		{
			std::cout << "CLEANED " << *entry.key << std::endl;
			entry.key.reset();
		}
		else
		{
			//std::cout << *entry.key << std::endl;
		}
	}
}

void VirtualMachine::Execute()
{
	if (m_Panic)return;
	globalFunc->opCode.push_back((uint8_t)InCode::NIL);
	globalFunc->opCode.push_back((uint8_t)InCode::RETURN);
	#if DEBUG
	std::cout << "FUNCTION: global" << std::endl;
	Debug(globalFunc->opCode, globalFunc->constants,globalVariables);
	for (auto& name : functionNames)
	{
		std::cout << "FUNCTION: " << *name << std::endl;
		auto func= globalVariables.Get(name->GetStringView())->value.AsFunc();
		Debug(func->opCode, func->constants,globalVariables);
	}
	#endif
	//return;

	if (mainFunc)
	{
		mainFunc->opCode.insert(mainFunc->opCode.begin(), (uint8_t)InCode::POP);
		callFrames[nextToCurrentCallFrame++].function = mainFunc;
	}
	callFrames[nextToCurrentCallFrame].function = globalFunc.get();
	callFrames[nextToCurrentCallFrame].stackIndex = -1;
	nextToCurrentCallFrame++;
	auto frame = &callFrames[nextToCurrentCallFrame-1];
	while (true)
	{
		// check if there are strings to be freed
		// it doesn't have to be here, only for debugging
		// usually memory is freed when we want to allocate new memory
		CollectStrings();
		auto inst = frame->function->opCode[frame->ip++];

		switch (static_cast<InCode>(inst))
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push_back(frame->function->constants[frame->function->opCode[frame->ip++]]);
			break;
		}
		case InCode::TRUE:
		{
			vmStack.push_back(ValueContainer{ true });
			break;
		}case InCode::NIL:
		{
			vmStack.push_back(ValueContainer{ -1});
			break;
		}
		case InCode::FALSE:
		{
			vmStack.push_back(ValueContainer{ false });
			break;
		}
		case InCode::ADD_FLOAT:
		{
			BINARY_OP(float, +);
			break;
		}
		case InCode::SUBSTRACT_FLOAT:
		{
			BINARY_OP(float, -);
			break;
		}
		case InCode::MULTIPLY_FLOAT:
		{
			BINARY_OP(float,*);
			break;
		}
		case InCode::DIVIDE_FLOAT:
		{
			BINARY_OP(float ,/ );
			break;
		}
		case InCode::ADD_INT:
		{
			BINARY_OP(int, +);
			break;
		}
		case InCode::ADD_STRING:
		{
			auto v = vmStack.back().AsString(); 
			vmStack.pop_back(); 
			auto v2 = vmStack.back().AsString();
			vmStack.pop_back(); 
			auto newString = VirtualMachine::AddStrings(v2,v);
			vmStack.push_back(ValueContainer{ newString }); \

			break;
		}
		case InCode::CAST_FLOAT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::FLOAT;
			float v = value.As<int>();
			value.as = v;
			break;
		}
		case InCode::CAST_INT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::INT;
			int v = value.As<float>();
			value.as = v;
			break;
		}
		case InCode::CAST_BOOL_INT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::BOOL;
			bool v = value.As<int>();
			value.as = v;
			break;
		}
		case InCode::CAST_BOOL_FLOAT:
		{
			auto& value = vmStack.back();
			value.type = ValueType::BOOL;
			bool v = value.As<float>();
			value.as = v;
			break;
		}
		case InCode::SUBSTRACT_INT:
		{
			BINARY_OP(int, -);
			break;
		}
		case InCode::MULTIPLY_INT:
		{
			BINARY_OP(int, *);
			break;
		}
		case InCode::DIVIDE_INT:
		{
			BINARY_OP(int, / );
			break;
		}
		case InCode::DIVIDE_PERCENT:
		{
			BINARY_OP(int, % );
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
				vmStack.push_back(ValueContainer{-value.As<float>()});
			}
			else if (value.type == ValueType::INT)
			{
				vmStack.push_back(ValueContainer{-value.As<int>() });
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
			vmStack.push_back(ValueContainer{ !value.As<bool>() });
			break;
		}
		case InCode::LESS_FLOAT:
		{
			BINARY_OP(float, < );
			break;
		}
		case InCode::GREATER_FLOAT:
		{
			BINARY_OP(float, > );
			break;
		}
		case InCode::LESS_INT:
		{
			BINARY_OP(int, < );
			break;
		}
		case InCode::GREATER_INT:
		{
			BINARY_OP(int, > );
			break;
		}

		case InCode::EQUAL_EQUAL:
		{
			auto  v2 = vmStack.back();
			vmStack.pop_back();
			auto  v1 = vmStack.back();
			vmStack.pop_back();
			vmStack.push_back(ValueContainer{ AreEqual(v1,v2) });
			break;
		}
		case InCode::AND:
		{
			BINARY_OP(bool, && );
			break;
		}
		case InCode::OR:
		{
			BINARY_OP(bool, ||);
			break;
		}
		case InCode::RETURN:
		{

			//if (globalFunc.get() == callFrames[nextToCurrentCallFrame - 1].function)
			//{
			//	return;
			//}
			auto prevCallFrameIndex = nextToCurrentCallFrame - 2;
			auto res = vmStack.back();
			if (prevCallFrameIndex < 0)
			{
				return;
			}
			if (callFrames[nextToCurrentCallFrame - 1].function == globalFunc.get())
			{
				vmStack.resize(0);
			}else
			vmStack.resize(callFrames[nextToCurrentCallFrame-1].stackIndex);
			vmStack.push_back(res);
			nextToCurrentCallFrame--;
			frame = &callFrames[prevCallFrameIndex];
			break;
		}
		case InCode::PRINT:
		{	auto& v = vmStack.back();
			std::cout << v << "\n";
			vmStack.pop_back();
			break;
		}
		case InCode::GET_GLOBAL_VAR:
		{	
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = (nameOfVariable.AsString())->GetStringView();
			auto entry = globalVariables.Get(string);
			vmStack.push_back(entry->value);
			break;
		}
		case InCode::SET_GLOBAL_VAR:
		{	
			auto& value = vmStack.back();
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = (nameOfVariable.AsString())->GetStringView();
			auto entry = globalVariables.Get(string);
			entry->value = value;
			vmStack.pop_back();
			break;
		}

		case InCode::GET_LOCAL_VAR:
		{
			auto index = frame->function->opCode[frame->ip++] + 1;
			vmStack.push_back(vmStack[frame->stackIndex+index]);
			break;
		}
		case InCode::SET_LOCAL_VAR:
		{
			auto index = frame->function->opCode[frame->ip++];
			auto value = vmStack.back();
			vmStack[index] = value;
			vmStack.pop_back();
			break;
		}

		case InCode::JUMP_IF_FALSE:
		{
			// if it is not false, then we should get to then block
			auto offset = frame->function->opCode[frame->ip++];
			auto condition = vmStack.back().As<bool>();
			if (!condition) frame->ip = (frame->ip-1) + (offset);
			break;
		}
		case InCode::JUMP:
		{
			auto offset = frame->function->opCode[frame->ip];
			frame->ip += offset;
			break;
		}
		case InCode::JUMP_BACK:
		{
			auto offset = frame->function->opCode[frame->ip];
			frame->ip -= offset;
			break;
		}
		case InCode::CALL:
		{
			// up to this point arguments and function are on stack
			auto argumentCount = frame->function->opCode[frame->ip++];
			auto funcIndex = vmStack.size()  - 1 - argumentCount;
			auto func = vmStack[funcIndex].AsFunc();
			auto newIndexFrame = CallFunction(func.get(), argumentCount, funcIndex);
			// update our call frame 
			frame = &callFrames[newIndexFrame];
			break;
		}
		case InCode::POP:
		{
			vmStack.pop_back();
			break;
		}
		default:
			break;
		}
	}
}

size_t VirtualMachine::CallFunction(Func* func, size_t argumentCount,size_t baseIndex)
{
	auto& frame = callFrames[nextToCurrentCallFrame++];
	frame.function = func;
	frame.ip = 0;
	frame.stackIndex = baseIndex;
	return nextToCurrentCallFrame - 1;

}
void VirtualMachine::GenerateBytecode(const Node const* node)
{
	currentFunc = globalFunc.get();

	try
	{
		Generate(node);
	}
	catch (const std::exception& e)
	{
		m_Panic = true;
		std::cout << e.what();
	}
	
}

#include "VirtualMachine.h"
#include "Incode.h"
#include "AST.h"
#include "Debug.h"
#include <string>
#include <cassert>
#include <algorithm>
#include <sstream>
#include "TokenConversion.h"
#include "Local.h"
#include "SSA.h"
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


ValueType DetermineOpTypeRet(ValueType type, InCode op, Func * currentFunc)
{
	switch (op)
	{
	case InCode::MULTIPLY:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::MULTIPLY_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::MULTIPLY_FLOAT);
			return ValueType::FLOAT;
		}
		break;

	case InCode::SUBSTRACT:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::SUBSTRACT_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::SUBSTRACT_FLOAT);
			return ValueType::FLOAT;
		}
		break;

	case InCode::ADD:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::ADD_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::ADD_FLOAT);
			return ValueType::FLOAT;
		}
		break;
	case InCode::DIVIDE:
		if (type == ValueType::INT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_INT);
			return ValueType::INT;
		}
		else if (type == ValueType::FLOAT)
		{
			currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_FLOAT);
			return ValueType::FLOAT;
		}
		break;
	default:
		assert(false && "Unknown operation");
		return ValueType::NIL;
	}

	// If none of the conditions match, return NIL as a fallback.
	return ValueType::NIL;
}


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

#define CAST_INT_FLOAT(type1,type2)\
if (type1== ValueType::INT && type2== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_FLOAT));\
}\
else if (type1== ValueType::FLOAT && type2== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_INT));\
}\


void VirtualMachine::CastWithDeclared(ValueType assignedType, ValueType declared)
{
	if (assignedType != declared && assignedType != ValueType::NIL)
	{
		if (declared == ValueType::INT)
		{
			currentFunc->opCode.push_back(((Bytecode)InCode::CAST_INT));
		}
		else
		{
			currentFunc->opCode.push_back(((Bytecode)InCode::CAST_FLOAT));
		}
	}
}


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

void VirtualMachine::ClearScope(const Scope* scope, StackSim& stackSim,
	std::vector<Bytecode>& opCode)
{
	int popAmount = scope->popAmount;
	while (popAmount > 0)
	{
		opCode.push_back((uint8_t)InCode::POP);
		popAmount--;
	}
	
	//if (stackSim.m_StackPtr == scopes.back()->popAmount)
	//{
	//	stackSim.m_StackPtr = 0;
	//	return;
	//}
	//if (stackSim.m_StackPtr < scopes.back()->popAmount)
	//{
	//	assert(false && "cannot pop so many elements");
	//}
	//for (int i = stackSim.m_StackPtr ; i > (stackSim.m_StackPtr - scopes.back()->popAmount); i-- )
	//{
	//	stackSim.locals[i-1] = stackSim.locals[i];
	//}
	//stackSim.m_StackPtr -= scopes.back()->popAmount;
	currentScope = currentScope->prevScope;
	//scopes.pop_back();
	//assert(stackSim.m_StackPtr >= 0);
}
ValueType VirtualMachine::GetGlobalType(const std::string& str,const Expression* const expr)
{
	auto entry = globalVariablesTypes.Get(str);
	if (!entry->IsInit())
	{
		std::stringstream ss;
		ss << "ERROR[" << (expr->line) << "]: " <<
			"The name " << str << " is used but not declared " << std::endl;
		throw std::exception{ ss.str().c_str() };
	}
	return entry->value.type;
}
ValueType VirtualMachine::GetLocalType(const std::string& str,const Expression* const expr)
{
	Entry* entry = currentScope->GetType(str);
	if (!entry->IsInit())
	{
		std::stringstream ss;
		ss << "ERROR[" << (expr->line) << "]: " <<
			"The name " << str << " is used but not declared " << std::endl;
		throw std::exception{ ss.str().c_str() };
	}
	return entry->value.type;
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
		auto type = GetGlobalType(str,expr);
		return type;
	}
	// local
	else if (expr->depth > 0)
	{
		auto str = expr->value.AsString();
		// check if it does exsist
		// maybe we should move it to ast tree, but the indexing will get 
		// compilcated since we don't know in indexing how much scopes we have passed
		auto [isDeclared,index,_] = currentScope->IsLocalExist(str, expr->depth);
	
		currentFunc->opCode.push_back((uint8_t)InCode::GET_LOCAL_VAR);
		//assert(index > 0 && "wrong index of local variable");
		currentFunc->opCode.push_back(index);

		auto type = GetLocalType(str,expr);
		return type;
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
		auto [isDeclared, index,_] = currentScope->IsLocalExist(str, expression->depth);
		currentFunc->opCode.push_back((uint8_t)InCode::SET_LOCAL_VAR);
		assert(index != -1);
		currentFunc->opCode.push_back(index);
	}
}
int VirtualMachine::GenerateLoopCondition(const Node* node)
{
	GenerateAST(node);
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
ValueType VirtualMachine::GetVariableType (const  std::string& name, int depthOfDeclaration)
{
	if (depthOfDeclaration > 0)
	{
		Entry* entry = nullptr;
		auto currentDepth = currentScope->depth;
		if (currentDepth == depthOfDeclaration)
		{
			entry = currentScope->types.Get(name);
		}
		else
		{
			auto diff = std::abs(currentDepth - depthOfDeclaration);
			Scope* prevScope = currentScope;
			while (diff > 0)
			{
				prevScope = prevScope->prevScope;
				diff--;
			}
			entry = prevScope->types.Get(name);
		}
		if (entry->IsInit()) return entry->value.type;
		assert(false && "could not get type of variable");
	}
	else
	{
		return globalVariablesTypes.Get(name)->value.type;
	}
}
ValueType VirtualMachine::GenerateAST(const Node * tree)
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
			 auto funcValue = globalVariables.Get(func->name)->
				 value.AsFunc();

			 funcValue->name = func->name;
			 if (func->name == "main")
			 {
				 mainFunc = funcValue.get();
			 }
			 currentFunc = funcValue.get();
			 auto prevScope = currentScope;
			 currentScope = const_cast<Scope*>(&func->paramScope);
			 currentScope->prevScope = prevScope;
			 for (auto& arg : func->arguments)
			 {
				 GenerateAST(arg.get());
			 }
			 GenerateAST(func->body.get());
			 auto type = globalVariablesTypes.Get(func->name)->value.type;
			 ClearLocal();
			 currentScope = prevScope;
			 //currentScope->stack.currentScopes.pop_back();
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
			 auto value = GenerateAST(exprLeft);
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
			 auto funcValue = globalVariables.Get(call->name)->value.AsFunc();
			 for (auto i = 0; i < call->args.size(); i++)
			 {
				 auto& arg = call->args[i];
				 auto argType = GenerateAST(arg.get());
				 auto declType = funcValue->argTypes[i];
				 CastWithDeclared(argType,declType);

			 }
			 currentFunc->opCode.push_back((uint8_t)InCode::CALL);
			 currentFunc->opCode.push_back(call->args.size());
			 auto type = globalVariablesTypes.Get(call->name)->value.type;
			 if (type == ValueType::NIL)
			 {
				 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 }
			 break;
		 }
		 case TokenType::BLOCK:
		 {
			 auto block = static_cast<Scope*>(const_cast<Node*>(tree));
			 // empty block
			 if (block->expressions.size() == 0)
			 {
				 return{};
			 }
			 auto prevScope = currentScope;
			 currentScope = const_cast<Scope*>(block);
			 currentScope->prevScope = prevScope;

			 for (auto& expression : block->expressions)
			 {
				 GenerateAST(expression.get());
			 }
			 // once we finish block, we must clear the stack
			 ClearScope(currentScope, currentScope->stack, currentFunc->opCode);
			 break;
		 }
		 case TokenType::PLUS:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 
			 if (left == right && left == ValueType::STRING)
			 {
				 currentFunc->opCode.push_back((Bytecode)InCode::ADD_STRING);
			 }
			 else
			 {
				return DetermineOpTypeRet(expr->value.type, InCode::ADD,currentFunc);
			 }
			 break;
		 }
		 case TokenType::PLUS_PLUS:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 DETERMINE_OP_TYPE(left, INCREMENT);
			 SetVariable(currentFunc->opCode, exprLeft);
			 return expr->value.type;
			 break;
		 }
		 case TokenType::MINUS_MINUS:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 DETERMINE_OP_TYPE(left, DECREMENT);
			 SetVariable(currentFunc->opCode, exprLeft);
			 return expr->value.type;
			 break;
		 }
		 case TokenType::STAR:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 return DetermineOpTypeRet(expr->value.type, InCode::MULTIPLY,currentFunc);
			 break;
		 }
		 case TokenType::MINUS:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 if (tree->As<Expression>()->right.get())
			 {
				 CAST_INT_FLOAT(left, expr->value.type);
				 auto right = GenerateAST(tree->As<Expression>()->right.get());
				 CAST_INT_FLOAT(right, expr->value.type);
				 return DetermineOpTypeRet(expr->value.type, InCode::SUBSTRACT, currentFunc);
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
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 return DetermineOpTypeRet(expr->value.type, InCode::DIVIDE, currentFunc);
			 break;
		 }
		 case TokenType::PERCENT:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT(left);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
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
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, GREATER);
			 return ValueType::BOOL;
			 }
		 case TokenType::GREATER_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, LESS);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
		}
		 case TokenType::EQUAL_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			 return ValueType::BOOL;
		}
		 case TokenType::BANG_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
		}
		 case TokenType::DECLARE:
		 {
			 // declaring a variable
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());
			 assert(tree->As<Expression>()->left.get() != nullptr);
			 auto str = exprLeft->value.AsString();
			 auto declType = GetVariableType(str, exprLeft->depth);
			 CastWithDeclared(expressionType, declType);


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
			 auto left = tree->As<Expression>()->left.get()->As<Expression>();
			 auto declType = GetVariableType(left->value.AsString(), left->depth);
			 // should not we invoke GetVariableType?
			 //if (left->depth == 0)
			 //{
			//	 declType = GetGlobalType(left->value.AsString(),left);
			 //}
			 //else
			 //{
			//	 declType = GetLocalType(left->value.AsString(),left);
			 //}
			 assert(declType != ValueType::NIL);
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());

			 CastWithDeclared(expressionType, declType);
			 SetVariable(currentFunc->opCode, exprLeft);
			 break;
			 }
		 case TokenType::PLUS_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());

			 CastWithDeclared(expressionType, left);


			 DETERMINE_OP_TYPE(left, ADD);

			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::STAR_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());


			 CastWithDeclared(expressionType, left);


			 DETERMINE_OP_TYPE(left, MULTIPLY);

			 SetVariable(currentFunc->opCode, exprLeft);

			 return exprLeft->value.type;
			 }
		 case TokenType::SLASH_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());


			 CastWithDeclared(expressionType, left);


			 DETERMINE_OP_TYPE(left, DIVIDE);


			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::MINUS_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 auto expressionType = GenerateAST(tree->As<Expression>()->right.get());


			 CastWithDeclared(expressionType, left);


			 DETERMINE_OP_TYPE(left, SUBSTRACT);


			 SetVariable(currentFunc->opCode, exprLeft);
			 return exprLeft->value.type;
			 }
		 case TokenType::LESS_EQUAL:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, GREATER);
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
			 }
		 case TokenType::LESS:
		 {
			 auto left = GenerateAST(tree->As<Expression>()->left.get());
			 CAST_INT_FLOAT(left, expr->value.type);
			 auto right = GenerateAST(tree->As<Expression>()->right.get());
			 CAST_INT_FLOAT(right, expr->value.type);
			 DETERMINE_BOOL(left, right, LESS);
			 return ValueType::BOOL;
			 }
		 case TokenType::AND:
		 {
			 GenerateAST(tree->As<Expression>()->left.get());
			 // check if it is false then we just ignore second operand
			 // and leave the value on stack
			 auto indexFalse = JumpIfFalse(currentFunc->opCode);
			 // remove the value because we didn't jump
			 // the whole and is dependent on second operand
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 GenerateAST(tree->As<Expression>()->right.get());
			 currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;
			 return ValueType::BOOL;
			 }
		 case TokenType::OR:
		 {
			 GenerateAST(tree->As<Expression>()->left.get());
			 auto indexFalse = JumpIfFalse(currentFunc->opCode);
			 // if it is true we get to jump
			 auto jump = Jump(currentFunc->opCode);

			 currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;

			 GenerateAST(tree->As<Expression>()->right.get());
			 // the first is true- just skip second operand
			 currentFunc->opCode.push_back((uint8_t)InCode::OR);
			 currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, jump) + 1;
			 return ValueType::BOOL;
			 }

		 case TokenType::BANG:
		 {
			 GenerateAST(tree->As<Expression>()->left.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			 return ValueType::BOOL;
			 }
		 case TokenType::PRINT:
		 {
			 GenerateAST(tree->As<Expression>()->left.get());
			 currentFunc->opCode.push_back((uint8_t)InCode::PRINT);
			 return ValueType::NIL;
			 }
		 case TokenType::IF:
		 {
			 GenerateAST(tree->As<Expression>()->left.get());
			 auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);

			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 GenerateAST(tree->As<Expression>()->right.get()->As<Expression>()->right.get());
			 auto indexJump = Jump(currentFunc->opCode);
			 currentFunc->opCode[indexJumpFalse] = (indexJump + 1) - indexJumpFalse;
			 // else branch
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 GenerateAST(tree->As<Expression>()->right.get()->As<Expression>()->left.get());
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
			 GenerateAST(body);
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

			 auto prevScope = currentScope;
			 currentScope = const_cast<Scope*>(&forNode->initScope);
			 currentScope->prevScope = prevScope;


			 GenerateAST(forNode->init.get());
			 auto firstIteration = Jump(currentFunc->opCode);
			 auto startLoopIndex = currentFunc->opCode.size();

			 BeginContinue(startLoopIndex);
			 auto prevSizeBreak = BeginBreak();

			 GenerateAST(forNode->action.get());
			 auto indexJumpFalse = GenerateLoopCondition(forNode->condition.get());
			 currentFunc->opCode[firstIteration] = CalculateJumpIndex(currentFunc->opCode, firstIteration) + 1;
			 GenerateAST(forNode->body.get());
			 EndContinue();

			 auto jump = JumpBack(currentFunc->opCode);
			 currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, startLoopIndex);

			 PatchBreak(prevSizeBreak);


			 // clean the check condition once we go finish the loop
			 currentFunc->opCode.push_back((uint8_t)InCode::POP);
			 currentFunc->opCode[indexJumpFalse] = CalculateJumpIndex(currentFunc->opCode, indexJumpFalse);
			 // because for loop has declared iterator variable
			 ClearScope(currentScope, currentScope->stack, currentFunc->opCode);
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


//std::shared_ptr<String> VirtualMachine::AddStrings(std::shared_ptr<String> s, std::shared_ptr<String> s1) 
//{
//	auto raw1 = s->GetRaw();
//	auto raw2 = s1->GetRaw();
//	auto newSize = s->GetSize() + s1->GetSize();
//	auto newString = new char[newSize + 1];
//	strcpy(newString, raw1);
//	strcat(newString, raw2);
//	auto res = AllocateString(newString, newSize);
//	// should move instead of  copy at the constructor of string
//	delete[] newString;
//	return res;
//
//}
//
//std::shared_ptr<String> VirtualMachine::AllocateString(const char* ptr, size_t size)
//{
//	if (internalStrings.IsExist(std::string_view{ ptr,size }))
//	{
//		auto str = internalStrings.Get(std::string_view{ ptr,size });
//		return (str->key);
//	}
//	auto* entry = internalStrings.Add(std::string_view{ptr,size});
//	return (entry->key);
//}


void VirtualMachine::ClearLocal()
{
	currentScope->stack.m_StackPtr = 0;
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
		return str == str2;
	}
	assert(false && "cannot compare the values of different type");
	return false;
}
//void VirtualMachine::CollectStrings() 
//{
//
//
//	//auto a = internalStrings.Get(std::string_view{"Hello, New Year!", 16});
//	//internalStrings.Print();
//	
//	//for (HashTable::Iterator i = internalStrings.begin(); i != internalStrings.end(); i++)
//	//{
//	//	std::cout << *i->key << std::endl;
//	//}
//	
//	for (auto& entry : internalStrings)
//	{
//		if (entry.key.use_count() == 1)
//		{
//			std::cout << "CLEANED " << *entry.key << std::endl;
//			entry.key.reset();
//		}
//		else
//		{
//			//std::cout << *entry.key << std::endl;
//		}
//	}
//}

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
		std::cout << "FUNCTION: " << name << std::endl;
		auto func= globalVariables.Get(name)->value.AsFunc();
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
		//CollectStrings();
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
			//auto newString = VirtualMachine::AddStrings(v2,v);
			auto newString = v2 + v;
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
			auto string = nameOfVariable.AsString();
			auto entry = globalVariables.Get(string);
			vmStack.push_back(entry->value);
			break;
		}
		case InCode::SET_GLOBAL_VAR:
		{	
			auto& value = vmStack.back();
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = nameOfVariable.AsString();
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

void VirtualMachine::GenerateCFG(const Block* block)
{
	for (const auto& instr : block->instructions)
	{
		auto type = instr.instrType;

		switch (type)
		{
		case TokenType::LEFT_PAREN:
			break;
		case TokenType::RIGHT_PAREN:
			break;
		case TokenType::LEFT_BRACE:
			break;
		case TokenType::RIGHT_BRACE:
			break;
		case TokenType::COMMA:
			break;
		case TokenType::DOT:
			break;
		case TokenType::DOUBLE_DOT:
			break;
		case TokenType::MINUS:
			break;
		case TokenType::PLUS:
			break;
		case TokenType::COLON:
			break;
		case TokenType::SEMICOLON:
			break;
		case TokenType::SLASH:
			break;
		case TokenType::STAR:
			break;
		case TokenType::PERCENT:
			break;
		case TokenType::BANG:
			break;
		case TokenType::BANG_EQUAL:
			break;
		case TokenType::EQUAL:
			break;
		case TokenType::EQUAL_EQUAL:
			break;
		case TokenType::PLUS_EQUAL:
			break;
		case TokenType::MINUS_EQUAL:
			break;
		case TokenType::SLASH_EQUAL:
			break;
		case TokenType::STAR_EQUAL:
			break;
		case TokenType::GREATER:
			break;
		case TokenType::GREATER_EQUAL:
			break;
		case TokenType::LESS:
			break;
		case TokenType::LESS_EQUAL:
			break;
		case TokenType::PLUS_PLUS:
			break;
		case TokenType::MINUS_MINUS:
			break;
		case TokenType::IDENTIFIER:
			break;
		case TokenType::INT_LITERAL:
			break;
		case TokenType::FLOAT_LITERAL:
			break;
		case TokenType::STRING_LITERAL:
			break;
		case TokenType::ERROR:
			break;
		case TokenType::END:
			break;
		case TokenType::AND:
			break;
		case TokenType::CLASS:
			break;
		case TokenType::ELSE:
			break;
		case TokenType::FALSE:
			break;
		case TokenType::CONTINUE:
			break;
		case TokenType::BREAK:
			break;
		case TokenType::FOR:
			break;
		case TokenType::FUN:
			break;
		case TokenType::IF:
			break;
		case TokenType::ELIF:
			break;
		case TokenType::NIL:
			break;
		case TokenType::OR:
			break;
		case TokenType::PRINT:
			break;
		case TokenType::RETURN:
			break;
		case TokenType::SUPER:
			break;
		case TokenType::THIS:
			break;
		case TokenType::TRUE:
			break;
		case TokenType::VAR:
			break;
		case TokenType::WHILE:
			break;
		case TokenType::STRING_TYPE:
			break;
		case TokenType::FLOAT_TYPE:
			break;
		case TokenType::INT_TYPE:
			break;
		case TokenType::BOOL_TYPE:
			break;
		case TokenType::BLOCK:
			break;
		case TokenType::DEDUCE:
			break;
		case TokenType::DECLARE:
			break;
		case TokenType::JUMP:
			break;
		case TokenType::BRANCH:
			break;
		case TokenType::PHI:
			break;
		default:
			break;
		}
	}


	for (auto child : block->blocks)
	{
		GenerateCFG(child);
	}
}

void VirtualMachine::GenerateBytecodeCFG(const CFG& cfg)
{
	currentFunc = globalFunc.get();
	GenerateCFG(cfg.startBlock);
}

void VirtualMachine::GenerateBytecodeAST(const Node const* node)
{
	currentFunc = globalFunc.get();
	try
	{
		GenerateAST(node);
	}
	catch (const std::exception& e)
	{
		m_Panic = true;
		std::cout << e.what();
	}
	
}

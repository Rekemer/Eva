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
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_INT); \
	return ValueType::INT; \
	}\
	else\
	{\
	currentFunc->opCode.push_back((uint8_t)InCode::OP##_FLOAT); \
	return ValueType::FLOAT; \
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
		auto str = expr->value.As<String*>();
		currentFunc->constants.emplace_back(str);
		currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
		auto entry = globalVariablesTypes.Get(str->GetStringView());
		return entry->value.type;
	}
	// local
	else if (expr->depth > 0)
	{
		auto str = expr->value.As<String*>();
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
	auto str = expression->value.As<String*>();;
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
ValueType VirtualMachine::Generate(const Node * tree)
{
		 if (!tree) return ValueType::NIL;
		 auto expr = static_cast<const Expression*>(tree);
		 auto exprLeft = static_cast<const Expression*>(tree->As<Expression>()->left.get());
		 if (tree->type == TokenType::FUN)
		 {
			 auto func = static_cast<const FunctionNode*>(tree);
			 auto funcValue = globalVariables.Add(func->name.GetStringView(),LiteralToType(func->type));

			 if (func->name == "main")
			 {
				 mainFunc = funcValue->value.As<Func*>();
			 }
			 currentFunc = funcValue->value.As<Func*>();
			 currentScopes.push_back(&func->paramScope);
			 for (auto& arg : func->arguments)
			 {
				 Generate(arg.get());
			 }
			 Generate(func->body.get());
			 ClearScope(currentScopes,m_StackPtr, currentFunc->opCode);
		 }
		 else if (tree->type == TokenType::LEFT_PAREN)
		 {
			 // invoke function
			 auto call = static_cast<const Call*>(tree);
			 currentFunc->opCode.push_back((uint8_t)InCode::GET_GLOBAL_VAR);
			 currentFunc->constants.emplace_back(&call->name);
			 for (auto& arg : call->args)
			 {
				 Generate(arg.get());
			 }
			 currentFunc->opCode.push_back((uint8_t)InCode::CALL);
			 currentFunc->opCode.push_back(call->args.size());
		 }
		 else if (tree->type == TokenType::BLOCK)
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

			SetVariable(currentFunc->opCode, exprLeft);
			return expr->value.type;

		}
		else if (tree->type == TokenType::MINUS_MINUS)
		{
			auto left = Generate(tree->As<Expression>()->left.get());



			DETERMINE_OP_TYPE(left, DECREMENT);


			SetVariable(currentFunc->opCode, exprLeft);
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
				currentFunc->opCode.push_back((uint8_t)InCode::NEGATE);
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
		else if (tree->type == TokenType::PERCENT)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT(left);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT(right);
			currentFunc->opCode.push_back((uint8_t)InCode::DIVIDE_PERCENT);
			return ValueType::INT;
		}
		else if (tree->type == TokenType::INT_LITERAL)
		{
			currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			currentFunc->constants.push_back(ValueContainer{ expr->value });
			currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			return ValueType::INT;
		}
		else if (tree->type == TokenType::FLOAT_LITERAL)
		{
			currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			currentFunc->constants.push_back(ValueContainer{ expr->value });
			currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			return ValueType::FLOAT;
		}
		else if (tree->type == TokenType::STRING_LITERAL)
		{
			currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
			// might copy because vector can reallocate
			currentFunc->constants.emplace_back(expr->value );
			currentFunc->opCode.push_back(currentFunc->constants.size() - 1); 
			return ValueType::STRING;
		}
		else if (tree->type == TokenType::IDENTIFIER)
		{
			return GetVariable(currentFunc->opCode, expr);
		}
		else if (tree->type == TokenType::TRUE)
		{
			currentFunc->opCode.push_back((uint8_t)InCode::TRUE);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::FALSE)
		{
			currentFunc->opCode.push_back((uint8_t)InCode::FALSE);
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
			currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::EQUAL_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto right = Generate(tree->As<Expression>()->right.get());
			currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::BANG_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto right = Generate(tree->As<Expression>()->right.get());
			currentFunc->opCode.push_back((uint8_t)InCode::EQUAL_EQUAL);
			currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::DECLARE)
		{
			// declaring a variable
			auto expressionType = Generate(tree->As<Expression>()->right.get());
			assert(tree->As<Expression>()->left.get() != nullptr);
			auto str = exprLeft->value.As<String*>();

			CAST_INT_FLOAT(expressionType, exprLeft->value.type);


			assert(exprLeft != nullptr);
			if (exprLeft->depth == 0)
			{
				currentFunc->constants.emplace_back(exprLeft->value);
				currentFunc->opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
				currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			}
		}
		else if (tree->type == TokenType::EQUAL)
		{
			// declaring a variable
			auto expressionType = Generate(tree->As<Expression>()->right.get());
			assert(tree->As<Expression>()->left.get()!= nullptr);

			CAST_INT_FLOAT(expressionType, exprLeft->value.type);
			SetVariable(currentFunc->opCode, exprLeft);

		}
		else if (tree->type == TokenType::PLUS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());

			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, ADD);

			SetVariable(currentFunc->opCode, exprLeft);
			return exprLeft->value.type;
		}
		else if (tree->type == TokenType::STAR_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, MULTIPLY);

			SetVariable(currentFunc->opCode, exprLeft);

			return exprLeft->value.type;
			}
		else if (tree->type == TokenType::SLASH_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, DIVIDE);


			SetVariable(currentFunc->opCode, exprLeft);
			return exprLeft->value.type;
		}
		else if (tree->type == TokenType::MINUS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			auto expressionType = Generate(tree->As<Expression>()->right.get());


			CAST_INT_FLOAT(expressionType, left);


			DETERMINE_OP_TYPE(left, SUBSTRACT);


			SetVariable(currentFunc->opCode, exprLeft);
			return exprLeft->value.type;
			}
		else if (tree->type == TokenType::LESS_EQUAL)
		{
			auto left = Generate(tree->As<Expression>()->left.get());
			CAST_INT_FLOAT(left, expr->value.type);
			auto right = Generate(tree->As<Expression>()->right.get());
			CAST_INT_FLOAT(right, expr->value.type);
			DETERMINE_BOOL(left, right, GREATER);
			currentFunc->opCode.push_back((uint8_t)InCode::NOT);
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
			auto indexFalse = JumpIfFalse(currentFunc->opCode);
			// remove the value because we didn't jump
			// the whole and is dependent on second operand
			currentFunc->opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get());
			currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::OR)
		{
			Generate(tree->As<Expression>()->left.get());
			auto indexFalse = JumpIfFalse(currentFunc->opCode);
			// if it is true we get to jump
			auto jump = Jump(currentFunc->opCode);
			
			currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse)+1;

			Generate(tree->As<Expression>()->right.get());
			// the first is true- just skip second operand
			currentFunc->opCode.push_back((uint8_t)InCode::OR);
			currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, jump)+1;
			return ValueType::BOOL;
		}
		
		else if (tree->type == TokenType::BANG)
		{
			Generate(tree->As<Expression>()->left.get());
			currentFunc->opCode.push_back((uint8_t)InCode::NOT);
			return ValueType::BOOL;
		}
		else if (tree->type == TokenType::PRINT)
		{
			Generate(tree->As<Expression>()->left.get());
			currentFunc->opCode.push_back((uint8_t)InCode::PRINT);
			return ValueType::NIL;
		}
		else if (tree->type == TokenType::IF)
		{
			Generate(tree->As<Expression>()->left.get());
			auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);

			currentFunc->opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get()->As<Expression>()->right.get());
			auto indexJump = Jump(currentFunc->opCode);
			currentFunc->opCode[indexJumpFalse] = (indexJump+1) - indexJumpFalse;
			// else branch
			currentFunc->opCode.push_back((uint8_t)InCode::POP);
			Generate(tree->As<Expression>()->right.get()->As<Expression>()->left.get());
			// once we execute then branch we need to skip else bytecode
			// without -1 because we need index of next bytecode, not previous one
			currentFunc->opCode[indexJump] = currentFunc->opCode.size()   - indexJump;

		}
		else if (tree->type == TokenType::WHILE)
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

		}
		else if (tree->type== TokenType::FOR)
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
		}
		else if (tree->type == TokenType::CONTINUE)
		{
			int index = JumpBack(currentFunc->opCode);
			assert(m_StartLoopIndexes.size() > 0);
			currentFunc->opCode[index] = CalculateJumpIndex(currentFunc->opCode, m_StartLoopIndexes.top());
		}
		else if (tree->type == TokenType::BREAK)
		{
			m_BreakIndexes.push(Jump(currentFunc->opCode));
		}
		else
		{
			// should say what type it is
			std::cout << "ERROR Code generation: weird type " << tokenToString(tree->type) << std::endl;
			assert(false);
		}

}

String* VirtualMachine::AllocateString(const char* ptr, size_t size)
{
	if (internalStrings.IsExist(std::string_view{ ptr,size }))
	{
		auto str = internalStrings.Get(std::string_view{ ptr,size });
		return (str->key);
	}
	auto* entry = internalStrings.Add(std::string_view{ptr,size}, ValueContainer{});
	return (entry->key);
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
		auto str = static_cast<String*>(a.As<String*>());
		auto str2 = static_cast<String*>(b.As<String*>());
		return *str == *str2;
	}
	return false;
}
void VirtualMachine::Execute()
{
	if (m_Panic)return;
	globalFunc->opCode.push_back((uint8_t)InCode::RETURN);
	#if DEBUG
	Debug(globalFunc->opCode,globalFunc->constants,globalVariables);
	#endif
	if (mainFunc)
	{
		callFrames[currentCallFrame].function = mainFunc;
	}
	else
	{
		callFrames[currentCallFrame].function = globalFunc.get();
	}

	auto frame = &callFrames[currentCallFrame];
	while (true)
	{
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
			auto&  v2 = vmStack.back();
			vmStack.pop_back();
			auto&  v1 = vmStack.back();
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
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = (nameOfVariable.As<String*>())->GetStringView();
			auto entry = globalVariables.Get(string);
			vmStack.push_back(entry->value);
			break;
		}
		case InCode::SET_GLOBAL_VAR:
		{	
			auto& value = vmStack.back();
			auto& nameOfVariable = frame->function->constants[frame->function->opCode[frame->ip++]];
			auto string = (nameOfVariable.As<String*>())->GetStringView();
			auto entry = globalVariables.Get(string);
			entry->value = value;
			vmStack.pop_back();
			break;
		}

		case InCode::GET_LOCAL_VAR:
		{
			auto index = frame->function->opCode[frame->ip++];
			vmStack.push_back(vmStack[index]);
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
		case InCode::POP:
		{
			vmStack.pop_back();
		}
		default:
			break;
		}
	}
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

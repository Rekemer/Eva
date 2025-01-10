#include  <iostream>
#include "Compiler.h"
#include "Lexer.h"
#include "AST.h"
#include "Function.h"
#include "InCode.h"
#include "SSA.h"
#include "Options.h"

#define CAST_INT_FLOAT(type1,type2)\
if (type1== ValueType::INT && type2== ValueType::FLOAT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_FLOAT));\
}\
else if (type1== ValueType::FLOAT && type2== ValueType::INT)\
{\
	currentFunc->opCode.push_back(((Bytecode)InCode::CAST_INT));\
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
// returns the index for backpatching
int JumpIfFalse(std::vector<Bytecode>&opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_IF_FALSE);
	// the address of else byteblock code - backpatching
	// then branch
	opCode.push_back((uint8_t)0);
	auto indexJumpFalse = opCode.size() - 1;
	return indexJumpFalse;
}
int Jump(std::vector<Bytecode>&opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
int JumpBack(std::vector<Bytecode>&opCode)
{
	opCode.push_back((uint8_t)InCode::JUMP_BACK);
	opCode.push_back((uint8_t)0);
	auto indexJump = opCode.size() - 1;
	return indexJump;
}
// from: to make jump relative
int CalculateJumpIndex(const std::vector<Bytecode> const& opCode, const int from)
{
	return opCode.size() - 1 - from;
}

void Compiler::CastWithDeclared(ValueType assignedType, ValueType declared)
{
	if (assignedType == declared) return;
	assert(assignedType == ValueType::INT || assignedType == ValueType::FLOAT);
	assert(declared == ValueType::INT || declared == ValueType::FLOAT);
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

ValueType DetermineOpTypeRet(ValueType type, InCode op, Func* currentFunc)
{
	assert(type != ValueType::NIL);
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
	case InCode::DIVIDE_PERCENT:
	{
		currentFunc->opCode.push_back((Bytecode)InCode::DIVIDE_PERCENT);
		return ValueType::INT;
		break;
	}
	default:
		assert(false && "Unknown operation");
		return ValueType::NIL;
	}

	// If none of the conditions match, return NIL as a fallback.
	return ValueType::NIL;
}
void EmitLocalSet(Func* currentFunc, int index)
{
	currentFunc->opCode.push_back((Bytecode)InCode::SET_LOCAL_VAR);
	assert(index != -1);
	currentFunc->opCode.push_back(index);
}
void GenerateLocalSet(Scope* currentScope, Func* currentFunc, const std::string& str, int depth)
{
	assert(currentScope != nullptr);
	auto [isDeclared, index, _] = currentScope->IsLocalExist(str, depth);
	EmitLocalSet(currentFunc, index);
}
void EmitGlobalSet(Func* currentFunc, const std::string& str)
{
	currentFunc->constants.emplace_back(str);
	currentFunc->opCode.push_back((uint8_t)InCode::SET_GLOBAL_VAR);
	currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
}
void EmitSet(Func* currentFunc, const Operand& variable)
{
	if (variable.depth > 0)
	{
		assert(variable.index != -1);
		EmitLocalSet(currentFunc, variable.index);
	}
	else EmitGlobalSet(currentFunc, variable.value.AsString());
}


void EmitPop(std::vector<Bytecode>&opCode)
{
	opCode.push_back((Bytecode)InCode::POP);
}
void EmitGlobalGet(Func * currentFunc, const std::string & str)
{
	currentFunc->opCode.push_back((Bytecode)InCode::GET_GLOBAL_VAR);
	currentFunc->constants.emplace_back(str);
	currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
}

void EmitLocalGet(Func * currentFunc, int index)
{
	currentFunc->opCode.push_back((Bytecode)InCode::GET_LOCAL_VAR);
	currentFunc->opCode.push_back(index);
}


void GenerateLocalGet(Scope * currentScope, Func * currentFunc, const std::string & str, int depth)
{
	// check if it does exsist
		// maybe we should move it to ast tree, but the indexing will get 
		// compilcated since we don't know in indexing how much scopes we have passed
	auto [isDeclared, index, _] = currentScope->IsLocalExist(str, depth);
	EmitLocalGet(currentFunc, index);
}



void EmitGet(Func * currentFunc, const Operand & variable)
{
	if (variable.IsTemp()) return;
	assert(variable.depth != -1);
	if (variable.depth > 0)
	{
		assert(variable.index != -1);
		EmitLocalGet(currentFunc, variable.index);
	}
	else EmitGlobalGet(currentFunc, variable.value.AsString());
}


void Compiler::BeginContinue(int startLoopIndex)
{
	m_StartLoopIndexes.push(startLoopIndex);
}
void Compiler::EndContinue()
{
	m_StartLoopIndexes.pop();
}
int Compiler::BeginBreak()
{
	return m_BreakIndexes.size();
}
void Compiler::PatchBreak(int prevSizeBreak)
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
ValueType Compiler::GetGlobalType(const std::string& str)
{
	auto entry = globalVariablesTypes.Get(str);
	if (!entry->IsInit())
	{
		//std::stringstream ss;
		//ss << "ERROR[" << (expr->line) << "]: " <<
		//	"The name " << str << " is used but not declared " << std::endl;
		auto mes = std::format("The name {} is used but not declared ", str);
		throw std::exception{ mes.c_str() };
	}
	return entry->value.type;
}
Block* Compiler::HandleBranch(std::vector<Block*> branches, const  Instruction& instr)
{
	std::vector<int> indexJumps;
	// condition
	auto& conditionOp = instr.operRight;
	GenerateCFGOperand(conditionOp, instr.returnType);

	auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);
	auto thenBlock = branches[0];
	EmitPop(currentFunc->opCode);
	// then
	//auto thenBlock = instr.targets.begin();
	GenerateBlockInstructions(thenBlock);
	auto indexJump = Jump(currentFunc->opCode);
	indexJumps.push_back(indexJump);
	currentFunc->opCode[indexJumpFalse] = (indexJump + 1) - indexJumpFalse;

	//elif
	for (int i = 1; i < branches.size() - 1; i += 1)
	{
		EmitPop(currentFunc->opCode);
		auto elseBlock = branches[i];
		// condition
		GenerateBlockInstructions(elseBlock);
		indexJumpFalse = JumpIfFalse(currentFunc->opCode);
		EmitPop(currentFunc->opCode);
		// elif then
		GenerateBlockInstructions(elseBlock->blocks[0]);
		indexJump = Jump(currentFunc->opCode);
		indexJumps.push_back(indexJump);
		currentFunc->opCode[indexJumpFalse] = (indexJump + 1) - indexJumpFalse;


		//auto elif = instr.targets.begin() + 1;
		// elif
		//auto elseBlock = instr.targets.end() - 1;

	}
	//else
	EmitPop(currentFunc->opCode);
	GenerateBlockInstructions(branches.back());
	auto mergeBlock = branches[0]->merge;
	for (auto index : indexJumps)
	{
		currentFunc->opCode[index] = currentFunc->opCode.size() - index;
	}

	if (mergeBlock != nullptr)
		GenerateBlockInstructions(mergeBlock);
	return mergeBlock;
}


void Compiler::GenerateCFGOperand(const Operand& operand, ValueType instrType)
{
	lastReturnType.push(instrType);
	auto type = operand.type;
	if (operand.isConstant)
	{
		GenerateConstant(operand.value);
	}
	else
	{
		EmitGet(currentFunc, operand);
	}
	CAST_INT_FLOAT(type, instrType);
}

void Compiler::GenerateConstant(const ValueContainer& v)
{
	if (v.type == ValueType::BOOL)
	{
		auto instr = v.As<bool>() ? (Bytecode)InCode::TRUE : (Bytecode)InCode::FALSE;
		currentFunc->opCode.push_back(instr);
		return;
	}
	currentFunc->opCode.push_back((uint8_t)InCode::CONST_VALUE);
	currentFunc->constants.push_back(v);
	currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
}


void Compiler::GenerateBlockInstructions(Block* block)
{
	if (block->isVisited) return;
	block->isVisited = true;

	auto tokenToInCodeOp = [](TokenType type)
		{
			switch (type)
			{
			case TokenType::STAR:
				return InCode::MULTIPLY;
			case TokenType::PLUS:
				return InCode::ADD;
			case TokenType::MINUS:
				return InCode::SUBSTRACT;
			case TokenType::SLASH:
				return InCode::DIVIDE;
			case TokenType::PERCENT:
				return InCode::DIVIDE_PERCENT;
			default:
				assert(false);
				break;
			}
		};
	for (const auto& instr : block->instructions)
	{
		auto type = instr.instrType;

		switch (type)
		{
		case TokenType::PUSH:
		{
			GenerateCFGOperand(instr.result, instr.result.value.type);
			break;
		}
		case TokenType::VAR:
		{
			auto exp = instr.operRight.value.type;
			auto real = instr.operLeft.value.type;
			auto& res = instr.result;
			GenerateCFGOperand(res, exp);
			//CAST_INT_FLOAT(real, exp);
			break;
		}
		case TokenType::LEFT_PAREN:
		{
			auto callName = instr.operRight.value.AsString();
			currentFunc->opCode.push_back((Bytecode)InCode::GET_GLOBAL_VAR);
			currentFunc->constants.emplace_back(callName);
			currentFunc->opCode.push_back(currentFunc->constants.size() - 1);
			auto argBlock = instr.argBlock;

			GenerateCFG(argBlock);
			//for (auto& arg : instr.variables)
			//{
			//	// doesn't do anything if we passed temp values, 
			//	// because they will be  on stack after parsing next SSA instructions
			//	GenerateCFGOperand(arg,instr.returnType);
			//}
			currentFunc->opCode.push_back((Bytecode)InCode::CALL);
			currentFunc->opCode.push_back(instr.operLeft.value.As<int>());
			if (instr.returnType == ValueType::NIL)
			{
				EmitPop(currentFunc->opCode);
			}
			break;
		}
		case TokenType::CALL:
		{
			assert(false);
			currentFunc->opCode.push_back((Bytecode)InCode::CALL);
			currentFunc->opCode.push_back(instr.variables.size());
			if (instr.returnType == ValueType::NIL)
			{
				EmitPop(currentFunc->opCode);
			}
			break;
		}
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
		case TokenType::BANG:
		{
			if (instr.IsUnary())
			{
				GenerateCFGOperand(instr.operRight, instr.returnType);
				if (type == TokenType::MINUS)
					currentFunc->opCode.push_back((Bytecode)InCode::NEGATE);
				else currentFunc->opCode.push_back((Bytecode)InCode::NOT);
				break;
			}
		}
		case TokenType::SLASH:
		case TokenType::STAR:
		case TokenType::PLUS:
		case TokenType::PERCENT:
		{
			ValueType left = instr.operLeft.type, right = instr.operRight.type;
			// we cannot simply generate bytecode for left and right,
			// because the right (temporary) value can end up beneath
			// of left operand, since it is result of previous computations
			// 
			// we could use swap instead ?
			if (instr.operRight.IsTemp())
			{
				if (!lastReturnType.empty())
				{
					auto prevRetType = lastReturnType.top();
					lastReturnType.pop();
					CAST_INT_FLOAT(prevRetType, instr.returnType);
				}
				currentFunc->opCode.push_back((Bytecode)InCode::STORE_TEMP);
				EmitPop(currentFunc->opCode);
				GenerateCFGOperand(instr.operLeft, instr.returnType);
				currentFunc->opCode.push_back((Bytecode)InCode::LOAD_TEMP);
			}
			else
			{
				GenerateCFGOperand(instr.operLeft, instr.returnType);
				GenerateCFGOperand(instr.operRight, instr.returnType);

			}



			if (left == right && left == ValueType::STRING)
			{
				currentFunc->opCode.push_back((Bytecode)InCode::ADD_STRING);
			}
			else
			{
				DetermineOpTypeRet(instr.returnType, tokenToInCodeOp(type), currentFunc);
			}
			break;
		}
		case TokenType::PLUS_PLUS:
		case TokenType::MINUS_MINUS:
		{
			assert(instr.returnType != ValueType::NIL);
			GenerateCFGOperand(instr.result, instr.returnType);
			assert(instr.result.type != ValueType::NIL);
			if (type == TokenType::PLUS_PLUS)
			{
				DETERMINE_OP_TYPE(instr.result.type, INCREMENT);
			}
			else if (type == TokenType::MINUS_MINUS)
			{
				DETERMINE_OP_TYPE(instr.result.type, DECREMENT);
			}
			EmitSet(currentFunc, instr.result);
			break;
		}
		case TokenType::COLON:
			break;
		case TokenType::SEMICOLON:
			break;
		case TokenType::DECLARE:
		case TokenType::EQUAL:
		{
			// we can have only one operand in our case, either a variable or constant
			auto res = instr.result;
			//const auto& varName = res.value.AsString() +"_" + std::to_string(res.version);
			const auto& varName = res.value.AsString();
			auto declType = res.type;
			assert(declType != ValueType::NIL);
			auto& assigned = instr.operRight;
			if (assigned.version == NOT_INIT_OPERAND) break;
			auto expressionType = assigned.type;
			if (assigned.isConstant)
			{
				GenerateConstant(assigned.value);
			}
			else
			{
				if (!assigned.IsTemp())
				{
					EmitGet(currentFunc, assigned);
				}
			}
			assert(expressionType != ValueType::NIL);
			CastWithDeclared(expressionType, declType);
			if (!res.IsTemp() && type == TokenType::EQUAL || type == TokenType::DECLARE
				&& res.depth == 0)
			{
				EmitSet(currentFunc, res);
			}
			break;
		}

		case TokenType::EQUAL_EQUAL:
		case TokenType::BANG_EQUAL:
		{
			auto castType = HighestType(instr.operLeft.type, instr.operRight.type);
			GenerateCFGOperand(instr.operLeft, castType);
			GenerateCFGOperand(instr.operRight, castType);
			currentFunc->opCode.push_back((Bytecode)InCode::EQUAL_EQUAL);
			if (type == TokenType::BANG_EQUAL)
			{
				currentFunc->opCode.push_back((Bytecode)InCode::NOT);
			}
		}
		break;
		case TokenType::GREATER:
		case TokenType::GREATER_EQUAL:
		{
			auto castType = HighestType(instr.operLeft.type, instr.operRight.type);
			GenerateCFGOperand(instr.operLeft, castType);
			GenerateCFGOperand(instr.operRight, castType);
			auto left = instr.operLeft.type;
			auto right = instr.operRight.type;
			if (type == TokenType::GREATER_EQUAL)
			{
				DETERMINE_BOOL(left, right, LESS);
				currentFunc->opCode.push_back((Bytecode)InCode::NOT);
			}
			else
			{
				DETERMINE_BOOL(left, right, GREATER);
			}
			break;
		}
		break;
		case TokenType::LESS:
		case TokenType::LESS_EQUAL:
		{
			auto castType = HighestType(instr.operLeft.type, instr.operRight.type);
			GenerateCFGOperand(instr.operLeft, castType);
			GenerateCFGOperand(instr.operRight, castType);
			auto left = instr.operLeft.type;
			auto right = instr.operRight.type;
			if (type == TokenType::LESS_EQUAL)
			{
				DETERMINE_BOOL(left, right, GREATER);
				currentFunc->opCode.push_back((Bytecode)InCode::NOT);
			}
			else
			{
				DETERMINE_BOOL(left, right, LESS);
			}
			break;
		}
		case TokenType::IDENTIFIER:
			break;
		case TokenType::ERROR:
			break;
		case TokenType::END:
			break;
		case TokenType::AND:
		{
			if (instr.operRight.IsTemp())
			{
				currentFunc->opCode.push_back((Bytecode)InCode::STORE_TEMP);
				EmitPop(currentFunc->opCode);
				GenerateCFGOperand(instr.operLeft, instr.returnType);
				auto indexFalse = JumpIfFalse(currentFunc->opCode);
				EmitPop(currentFunc->opCode);
				currentFunc->opCode.push_back((Bytecode)InCode::LOAD_TEMP);
				currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;
			}
			else
			{
				GenerateCFGOperand(instr.operLeft, instr.returnType);
				auto indexFalse = JumpIfFalse(currentFunc->opCode);
				EmitPop(currentFunc->opCode);
				GenerateCFGOperand(instr.operRight, instr.returnType);
				currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;
			}

			break;
		}
		case TokenType::OR:
		{

			GenerateCFGOperand(instr.operLeft, instr.returnType);
			auto indexFalse = JumpIfFalse(currentFunc->opCode);
			// if it is true we get to jump
			auto jump = Jump(currentFunc->opCode);

			currentFunc->opCode[indexFalse] = CalculateJumpIndex(currentFunc->opCode, indexFalse) + 1;

			GenerateCFGOperand(instr.operRight, instr.returnType);
			// the first is true- just skip second operand
			currentFunc->opCode.push_back((uint8_t)InCode::OR);
			currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, jump) + 1;
			break;
		}
		case TokenType::CLASS:
			break;
		case TokenType::ELSE:
			break;
		case TokenType::FALSE:
			break;
		case TokenType::FOR:
			break;
		case TokenType::FUN:
		{
			//auto args = block->blocks[0];
			//auto body = block->blocks[1];
			auto funcName = instr.operRight.value.AsString();
			auto args = functionCFG->at(funcName).start;
			auto funcValue = globalVariables.Get(funcName)->
				value.AsFunc();
			funcValue->name = funcName;
			functionNames.push_back(funcValue->name);
			if (funcName == "main")
			{
				mainFunc = funcValue.get();
			}
			auto prevFunc = currentFunc;
			currentFunc = funcValue.get();
			m_FuncReturnType = globalVariablesTypes.Get(funcValue->name)->value.type;
			GenerateCFG(args);
			//ClearLocal();

			if (m_FuncReturnType == ValueType::NIL)
			{
				currentFunc->opCode.push_back((uint8_t)InCode::NIL);
				currentFunc->opCode.push_back((uint8_t)InCode::RETURN);
			}
			currentFunc = prevFunc;
			break;
		}
		case TokenType::IF:
			break;
		case TokenType::ELIF:
			break;
		case TokenType::NIL:
			break;
		case TokenType::PRINT:
		{
			auto& argument = instr.operRight;
			if (argument.isConstant)
			{
				GenerateConstant(argument.value);
			}
			else
			{
				if (!argument.IsTemp())
				{
					EmitGet(currentFunc, argument);
				}
			}
			currentFunc->opCode.push_back((uint8_t)InCode::PRINT);
			break;
		}
		case TokenType::RETURN:
		{
			if (m_FuncReturnType != ValueType::NIL)
			{
				auto popAmount = instr.operLeft.value.As<int>();

				GenerateCFGOperand(instr.operRight, m_FuncReturnType);
				currentFunc->opCode.push_back((Bytecode)InCode::STORE_TEMP);
				currentFunc->opCode.push_back((Bytecode)InCode::POP);
				for (int i = 0; i < popAmount; i++)
					EmitPop(currentFunc->opCode);
				currentFunc->opCode.push_back((Bytecode)InCode::LOAD_TEMP);
				currentFunc->opCode.push_back((Bytecode)InCode::RETURN);
			}
			break;
		}
		case TokenType::SUPER:
			break;
		case TokenType::THIS:
			break;
		case TokenType::TRUE:
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
		{
			auto popAmmount = instr.operRight.value.As<int>();
			for (int i = 0; i < popAmmount; i++)
			{
				currentFunc->opCode.push_back((Bytecode)InCode::POP);
			}
		}
		break;
		case TokenType::DEDUCE:
			break;
		case TokenType::JUMP_BRANCH:
			break;
		case TokenType::JUMP:
		{

			auto target = instr.targets[0];
			if (!target->isVisited)
				GenerateBlockInstructions(target);
			break;
		}
		case TokenType::JUMP_WHILE:
		{

			auto target = instr.targets[0];

			if (!target->isVisited)
			{
				conditionIndex.push(currentFunc->opCode.size());
				GenerateBlockInstructions(target);
			}
			break;
		}
		case TokenType::BRANCH:
		{

			//auto & branches = block->blocks;
			auto& branches = instr.targets;
			auto mergeBlock = HandleBranch(branches, instr);
			break;
		}
		case TokenType::JUMP_FOR:
		{
			//assert(instr.targets.size() < 4);
			auto firstIteration = Jump(currentFunc->opCode);
			auto startIndex = currentFunc->opCode.size();
			BeginContinue(startIndex);
			auto prevSizeBreak = BeginBreak();
			// action
			GenerateBlockInstructions(instr.targets[0]);

			// generate loop condition
			GenerateBlockInstructions(instr.targets[1]);
			auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);
			EmitPop(currentFunc->opCode);

			currentFunc->opCode[firstIteration] = CalculateJumpIndex(currentFunc->opCode, firstIteration) + 1;
			// body
			GenerateBlockInstructions(instr.targets[2]);



			EndContinue();

			auto jump = JumpBack(currentFunc->opCode);
			// jumping backwards
			currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, startIndex);

			PatchBreak(prevSizeBreak);

			EmitPop(currentFunc->opCode);
			currentFunc->opCode[indexJumpFalse] = CalculateJumpIndex(currentFunc->opCode, indexJumpFalse);
			GenerateBlockInstructions(instr.targets[3]);

			//ClearScope(currentScope, currentScope->stack, currentFunc->opCode);
			break;
		}
		case TokenType::BRANCH_WHILE:
		{
			assert(!conditionIndex.empty());
			auto startIndex = conditionIndex.top();
			auto indexJumpFalse = JumpIfFalse(currentFunc->opCode);
			EmitPop(currentFunc->opCode);

			BeginContinue(startIndex);
			auto prevSizeBreak = BeginBreak();

			auto body = instr.targets[0];
			GenerateBlockInstructions(body);


			auto jump = JumpBack(currentFunc->opCode);
			// jumping backwards
			currentFunc->opCode[jump] = CalculateJumpIndex(currentFunc->opCode, startIndex);

			EndContinue();
			PatchBreak(prevSizeBreak);

			EmitPop(currentFunc->opCode);
			currentFunc->opCode[indexJumpFalse] = CalculateJumpIndex(currentFunc->opCode, indexJumpFalse);
			conditionIndex.pop();
			GenerateBlockInstructions(instr.targets[1]);
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
		case TokenType::BRANCH_ELIF:
		{
			if (instr.operRight.isConstant)
			{
				GenerateConstant(instr.operRight.value);
			}
			break;
		}
		case TokenType::PHI:
		case TokenType::JUMP_BACK:
			break;
		default:
			assert(false);
			break;
		}
	}

}

void Compiler::GenerateCFG(Block* block)
{

	GenerateBlockInstructions(block);
	for (auto child : block->blocks)
	{
		if (!child->isVisited)
			GenerateCFG(child);
	}

	if (block->merge && !block->merge->isVisited)
	{
		GenerateCFG(block->merge);
	}
}

void Compiler::GenerateBytecodeCFG(const CFG& cfg)
{
	currentFunc = globalFunc.get();
	functionCFG = &cfg.functionCFG;
	GenerateCFG(cfg.functionCFG.at(cfg.currentFunc).start);
}

int Compiler::Compile(const char* line)
{
	Lexer parser;
	if (!parser.Parse(line)) return { -1};
	auto& tokens = parser.GetTokens();
#if DEBUG_TOKENS
	for (auto token : tokens)
	{
		std::cout << tokenToString(token.type) << " ";
	}
	std::cout << "\n";
#endif // DEBUG
	std::vector<AST> trees;
	auto ptr = tokens.begin();
	bool panic = false;
	
	while (ptr->type!=TokenType::END)
	{
		AST tree;
		tree.compiler = this;
		tree.Build(ptr);
		if (tree.IsPanic())
		{
			panic = true;
			continue;
		}
		tree.TypeCheck();
		if (tree.IsPanic())
		{
			panic = true;
			continue;
		}
#if CONSTANT_FOLD
		tree.Fold();
#endif
		trees.push_back(std::move( tree));

	}

	if (panic)
	{
		return -1;
	}
#if SSA
	CFG cfg;
	cfg.compiler = this;
	for (auto& tree : trees)
	{
		auto node = tree.GetTree();
		cfg.ConvertAST(node);
	}
	cfg.TopSort();
	cfg.BuildDominatorTree();
	cfg.BuildDF();
	cfg.InsertPhi();
#if CONST_PROP
	cfg.ConstPropagation();
#endif
#if DEC
	cfg.DeadCode();
#endif
#if DEBUG_SSA 

	cfg.Debug();
#endif 

	//return {};
#endif 

#if SSA
	GenerateBytecodeCFG(cfg);
#else
	for (auto& tree : trees)
	{
		vm.GenerateBytecodeAST(tree.GetTree());
	}
#endif
	return 0;
//	vm.Execute();
//#if DEBUG_STACK
//	std::cout << "STACK [ ";
//	auto& stack = vm.GetStack();
//	for (auto& v : stack)
//	{
//		std::cout << v << ", ";
//	}
//	std::cout << " ]\n";
//#endif // DEBUG
//	if (vm.GetStack().size() > 1)
//	{
//		auto& stack = vm.GetStack();
//		auto res = stack[stack.size()-2];
//		return { res,vm };
//	}
//	return { {},vm };
}


#include "SSA.h"
#include "AST.h"
#include "VirtualMachine.h"
#include "Tokens.h"
#include <cassert>
#include <queue>
#include <unordered_set>

std::ostream& operator<<(std::ostream& os, const Instruction& v)
{
	os << v.result.name << " = ";
	if (v.operLeft.name != "null") os << v.operLeft.name << " ";
	os << tokenToString(v.instrType) << " ";
	if (v.operRight.name != "null") os << v.operRight.name << " ";
	if (v.targets.size() > 0)
	{
		for (auto block : v.targets)
		{
			std::cout << block->name << " ";
		}
	}
	std::cout << std::endl;
	return os;
}

bool CFG::IsStatement(const Node* node)
{
	auto nodeType = node->type;
	// statement doesn't not return value: print, loops, if, else
	switch (nodeType)
	{
	case TokenType::LEFT_PAREN:
	{
		// we need to check whether is returns any value
		auto call = node->As<Call>();
		auto type = vm->GetGlobalsType().Get(call->name->GetStringView())->value.type;
		if (type == ValueType::NIL)
		{
			return true;
		}
		return false;
		break;
	}
	case TokenType::LEFT_BRACE:
	case TokenType::ELSE:
	case TokenType::CONTINUE:
	case TokenType::BREAK:
	case TokenType::FOR:
	case TokenType::FUN:
	case TokenType::IF:
	case TokenType::ELIF:
	case TokenType::PRINT:
	case TokenType::RETURN:
	case TokenType::WHILE:
	case TokenType::BLOCK:
		return true;
	default:
		break;
	}
	return false;
}
Block* CFG::CreateBlock(const std::string& name)
{
	Block* block = &graph[name];
	block->name = name;
	return block;
}
int GetVersion(std::unordered_map<String, int>& map, String& name)
{
	int version = -1;
	auto iter = map.find(name);
	if (iter != map.end())
	{
		version = iter->second++;
	}
	else version = map[name]++;
	return version;
}

void CFG::ConvertStatementAST(const Node* tree)
{
	auto type = tree->type;
	auto expr = tree->As<Expression>();
	switch (type)
	{
	case TokenType::LEFT_PAREN:
	{
		break;
	}
	case TokenType::DECLARE:
	{
		CreateVariable(tree);
		break;
	}
	case TokenType::EQUAL:
	{
		CreateVariable(tree);
		break;
	}
	case TokenType::ELSE:
	{
		break;
	}
	case TokenType::CONTINUE:
	case TokenType::BREAK:
	case TokenType::FOR:
	case TokenType::FUN:
	case TokenType::IF:
	{
		auto flows = expr->right.get();
		// we need to convert the condition
		auto condition = expr->left.get();
		auto valueCondition = ConvertExpressionAST(condition);
		
		

		createBlock = false;
		
		// then
		std::stringstream thenBlockName;
		thenBlockName << "[then_" <<  std::to_string(Block::counterThen++) << "]";
		auto then = CreateBlock(thenBlockName.str());
		auto prevblock = currentBlock;

		currentBlock = then;
		std::stringstream mergeName;
		mergeName << "[merge_" << Block::counterMerge << "]";
		ConvertStatementAST(flows->As<Expression>()->right.get());
		currentBlock->instructions.
			push_back(Instruction{ TokenType::JUMP,{},{mergeName.str()},{}});
		
		prevblock->blocks.push_back(then);
		// handle elif cases

		// else
		std::stringstream elseBlockName;
		elseBlockName << "[else_" << std::to_string(Block::counterElse++) << "]";
		auto els = CreateBlock(elseBlockName.str());
		currentBlock = els;
		ConvertStatementAST(flows->As<Expression>()->left.get());
		currentBlock->instructions.
			push_back(Instruction{ TokenType::JUMP,{},{mergeName.str()},{}});
		
		prevblock->blocks.push_back(els);

		//Instruction branch;
		auto branchInstr = Instruction{ TokenType::BRANCH,{},{valueCondition},{} };
		
		branchInstr.targets.push_back(then);
		branchInstr.targets.push_back(els);

		prevblock->instructions.push_back(branchInstr);


		auto merge = CreateBlock(mergeName.str());
		// phi function

		then->blocks.push_back(merge);
		els->blocks.push_back(merge);
		
		currentBlock = merge;

		break;
	}
	case TokenType::ELIF:

	case TokenType::PRINT:
	{
		auto value  = ConvertExpressionAST(expr->left.get());
		Instruction instr{ type,{},value,{} };
		currentBlock->instructions.push_back(instr);
		break;
	}
	case TokenType::RETURN:
	case TokenType::WHILE:
	case TokenType::BLOCK:
	{
		Block* block = currentBlock;
		//if (createBlock)
		//{
		//
		//	createBlock = false;
		//	auto name = std::string{ "Block_" } + std::to_string/(Block::counterStraight/++);
		//	currentBlock = block = CreateBlock(name);
		//	
		//}
		//auto previousBlock = currentBlock;
		//currentBlock = block;
		//if (Block::counterStraight == 1)
		//{
		//	startBlock = currentBlock;
		//}
		//else
		//{
		//	previousBlock->blocks.push_back(currentBlock);
		//}
		auto scope = tree->As<Scope>();
		for (auto& statement : scope->expressions)
		{
			ConvertStatementAST(statement.get());
		}
		break;
	}
	case TokenType::DEDUCE:
		assert(false && "not all types are deduced");
	default:
		break;
	}
}

void PrintBlock(Block* block)
{
	std::cout  << std::endl << block->name << std::endl;
	//std::cout << "------------" << std::endl;
	for (auto& instr : block->instructions)
	{
		std::cout << instr;
	}
	//std::cout << std::endl << "------------" << std::endl;
}

void CFG::Debug()
{
	// just current block for now
	// once we add control flow we should traverse blocks
	assert(startBlock != nullptr);

	std::queue<Block*> blockQueue;
	std::unordered_set<Block*> visitedBlocks;

	blockQueue.push(startBlock);
	visitedBlocks.insert(startBlock);

	while (!blockQueue.empty())
	{
		auto blockToPrint = blockQueue.front(); 
		blockQueue.pop();
		PrintBlock(blockToPrint);

		for (auto block : blockToPrint->blocks)
		{
			if (visitedBlocks.find(block) == visitedBlocks.end())
			{
				blockQueue.push(block);
				visitedBlocks.insert(block);
			}
		}

	}
}
void CFG::CreateVariable(const Node* tree)
{
	auto expr = tree->As<Expression>();
	// left is variable
	auto left = expr->left->As<Expression>();
	auto name = left->value.AsString();
	int version = -1;
	// get a version of a variable
	// local variable
	if (left->depth > 0)
	{
		version = GetVersion(localVariables, *name);
	}
	// global variable
	else
	{
		version = GetVersion(globalVariables, *name);
	}
	assert(version != -1);
	std::stringstream ss;
	ss << std::string(name->GetRaw());
	ss << "_";
	ss << std::to_string(version);
	Operand resOp{ ss.str() ,false,version };
	auto rightOp = ConvertExpressionAST(expr->right.get());

	auto instruction = Instruction{ tree->type,{},rightOp,resOp };
	currentBlock->instructions.push_back(instruction);

}

Operand CFG::BinaryInstr(const Expression* expr, TokenType type)
{
	auto left = ConvertExpressionAST(expr->left.get());
	auto right = ConvertExpressionAST(expr->right.get());
	auto version = GetTempVersion();
	std::stringstream resultName;
	resultName << "t_";
	resultName << version;
	auto res = Operand{ resultName.str(),false,version };

	Instruction instr{ type,left,right,res };
	currentBlock->instructions.push_back(instr);
	return res;
}

Operand CFG::ConvertExpressionAST(const Node* tree)
{
	auto type = tree->type;
	auto expr = tree->As<Expression>();
	
	switch (type)
	{

	case TokenType::IDENTIFIER:
	{
		auto value = expr->value.AsString();
		Operand op{ value->GetRaw(),false,0 };
		return op;
		break;
	}
	case TokenType::PLUS:
	case TokenType::STAR:
	case TokenType::MINUS:
	case TokenType::SLASH:
	{
		auto res = BinaryInstr(expr,type);
		return res;
		break;
	}
	case TokenType::PLUS_PLUS:
	{
		break;
	}
	case TokenType::MINUS_MINUS:
	{
		break;
	}
	
	case TokenType::PERCENT:
	{
		break;
	}
	case TokenType::INT_LITERAL:
	{
		auto number = expr->value.As<int>();
		Operand op{ std::to_string(number),true,0 };
		return op;
		break;
	}
	case TokenType::FLOAT_LITERAL:
	{
		auto number = expr->value.As<float>();
		Operand op{ std::to_string(number),true,0 };
		return op;
		break;
	}
	case TokenType::STRING_LITERAL:
	{
		break;
	}
	case TokenType::TRUE:
	case TokenType::FALSE:
	{
		auto value = expr->value.As<bool>();
		auto str = value ? "true" : "false";
		Operand op{ str,true,0 };
		return op;
		break;
	}
	case TokenType::GREATER:
	{
		break;
	}
	case TokenType::GREATER_EQUAL:
	{
		break;
	}
	case TokenType::EQUAL_EQUAL:
	{
		break;
	}
	case TokenType::BANG_EQUAL:
	{
		break;
	}

	
	case TokenType::EQUAL:
	{

		break;
	}
	case TokenType::PLUS_EQUAL:
	{
		break;
	}
	case TokenType::STAR_EQUAL:
	{
		break;
	}
	case TokenType::SLASH_EQUAL:
	{
		break;
	}
	case TokenType::MINUS_EQUAL:
	{
		break;
	}
	case TokenType::LESS_EQUAL:
	{
		break;
	}
	case TokenType::LESS:
	{
		break;
	}
	case TokenType::AND:
	{
		break;
	}
	case TokenType::OR:
	{
		break;
	}

	case TokenType::BANG:
	{
		break;
	}
	default:
		assert(false && "Error at AST to SSA expression conversion");
		break;
	}
	return {};
}
void CFG::ConvertAST(const Node* tree)
{
	auto expr = tree->As<Expression>();
	auto type = tree->type;

	startBlock  = currentBlock= CreateBlock("[block_0]");
	Block::counterStraight++;
	if (IsStatement(tree))
	{
		ConvertStatementAST(tree);
	}
	else
	{
		assert(false && "We are not supposed to have non statement at this level?");
	}
	
	
}
#include "SSA.h"
#include "AST.h"
#include "VirtualMachine.h"
#include "Tokens.h"
#include <cassert>
#include <queue>
#include <unordered_set>

//https://www.cs.cornell.edu/courses/cs6120/2023fa/lesson/6/

std::ostream& operator<<(std::ostream& os, const Instruction& v)
{
	os << v.result.name << "_" <<  v.result.version << " = ";
	if (v.operLeft.name != "null") os << v.operLeft.name << "_" << v.operLeft.version;
	os << tokenToString(v.instrType) << " ";
	if (v.instrType == TokenType::PHI)
	{
		os << "( ";
		for (auto& param : v.variables)
		{
			os << param.name << "_" << param.version << ", ";
		}
		os << ") ";
	}
	if (v.operRight.name != "null") os << v.operRight.name << "_" << v.operRight.version;
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
Block* CFG::CreateBlock(const std::string& name, std::vector<Block*> parents)
{
	Block* block = &graph[name];
	block->name = name;
	block->parents = parents;
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
	if (!tree) return;
	auto type = tree->type;
	auto expr = tree->As<Expression>();
	switch (type)
	{
	case TokenType::LEFT_PAREN:
	{
		break;
	}
	case TokenType::DECLARE:
	case TokenType::EQUAL:
	{
		CreateVariable(tree);
		break;
	}
	case TokenType::ELSE:
	{
		assert(false && "we should be here");
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
		auto parentBlock = currentBlock;
		
		//Instruction branch;
		auto branchInstr = Instruction{ TokenType::BRANCH,{},{valueCondition},{} };

		createBlock = false;
		
		// then
		std::stringstream thenBlockName;
		thenBlockName << "[then_" <<  std::to_string(Block::counterThen++) << "]";
		auto then = CreateBlock(thenBlockName.str(), { parentBlock });

		currentBlock = then;
		auto thenBranch = flows->As<Expression>()->right.get();
		std::stringstream mergeName;
		mergeName << "[merge_" << Block::counterMerge << "]";
		ConvertStatementAST(thenBranch);
		currentBlock->instructions.
			push_back(Instruction{ TokenType::JUMP,{},{mergeName.str()},{}});
		
		parentBlock->blocks.push_back(then);
		branchInstr.targets.push_back(then);
		// handle elif cases

		// else
		auto elseBranch = flows->As<Expression>()->left.get();
		std::stringstream elseBlockName;
		elseBlockName << "[else_" << std::to_string(Block::counterElse++) << "]";
		auto els = CreateBlock(elseBlockName.str(), { parentBlock });
		currentBlock = els;
		ConvertStatementAST(elseBranch);
		currentBlock->instructions.
			push_back(Instruction{ TokenType::JUMP,{},{mergeName.str()},{}});
		
		parentBlock->blocks.push_back(els);
		branchInstr.targets.push_back(els);

		
		
		
	

		parentBlock->instructions.push_back(branchInstr);

		auto merge = CreateBlock(mergeName.str(), {then,els});
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
	std::cout << "[dominators]\n[ ";

	for (auto dominator : block->dom)
	{
		if (dominator != block)
		std::cout << dominator->name << " ";
	}
	std::cout << "]" << std::endl;

	if (block->idom)
	{
		std::cout << "[immediate dominator] " << block->idom->name << std::endl;
	}

	if (block->df.size() > 0)
	{
		std::cout << "[dominance frontier] ";
		for (auto b : block->df)
		{
			std::cout << b->name;
		}
		std::cout << std::endl;
	}

	//std::cout << std::endl << "------------" << std::endl;
}
int CFG::NewName(std::string& name)
{
	auto version = variableCounterLocal[name]++;
	variableStack[name].push(version);
	return version;
}

void CFG::Rename(Block* b)
{

	// update result of phi
	for (auto index : b->phiInstructionIndexes)
	{
		auto& phi = b->instructions[index];
		phi.result.version = NewName(phi.result.name);
	}

	// update each operation
	for (auto& instr : b->instructions)
	{
		if (instr.instrType == TokenType::PHI) continue;
		if (instr.operLeft.isVariable())
		{
			instr.operLeft.version = variableStack[instr.operLeft.name].top();
		}
		if (instr.operRight.isVariable())
		{
			instr.operRight.version = variableStack[instr.operRight.name].top();
		}
		instr.result.version = NewName(instr.result.name);
	}
	// update phi parametrs
	for (auto child : b->blocks)
	{
		for (auto index : child->phiInstructionIndexes)
		{
			auto& phi = child->instructions[index];
			auto& varName = phi.result.name;
			if (variableStack[varName].size() > 0)
			{
				// we cannot simply iterate over stack
				// because the parents of merge node 
				// have not been proccesed yet
				auto parent = std::find(child->parents.begin(), child->parents.end(), b);
				if (parent == child->parents.end())
				{
					assert(false && "parent is not found!? ");
				}
				auto index = std::distance(child->parents.begin(),parent);
				phi.variables[index].version = variableStack[varName].top();
				phi.variables[index].name = varName;
			}

		}
	}

	for (auto s : b->dominatorChildren)
	{
		Rename(s);
	}

	// Pop variable versions for variables defined in this block
	for (auto it = b->instructions.rbegin(); it != b->instructions.rend(); ++it)
	{
		if (it->variables.size() > 0) continue;

		if (!it->result.isVariable())
		{
			auto& varName = it->result.name;
			variableStack[varName].pop();
		}
	}


	for (auto index : b->phiInstructionIndexes)
	{
		auto& phi = b->instructions[index];
		variableStack[phi.result.name].pop();
	}
	return;

}

void CFG::InsertPhi()
{
	// placing stage
	for (auto v : localAssigned)
	{
		// because one block can be in multiple dominance frontiers
		// we need to track whether the phi is placed already
		std::unordered_set<Block*> hasAlready;
		// we will update our workList hence a copy
		std::queue<Block*> workList;
		for (auto b : v.second)
		{
			workList.push(b);
		}

		// blocks that define v variable
		while (!workList.empty())
		{
			auto block = workList.front();
			workList.pop();
			for (auto blockDf : block->df)
			{
				if (hasAlready.find(blockDf) == hasAlready.end())
				{
					hasAlready.insert(blockDf);
					auto& str = v.first;
					std::string name = str.GetRaw();
					// Operand{name} is result  -  x = phi x x x x 
					auto instr = Instruction{ TokenType::PHI ,{},{},Operand{name}};
					auto potentialChanges = blockDf->parents.size();
					instr.variables.resize(potentialChanges);
					blockDf->instructions.insert(blockDf->instructions.begin(), instr);
					blockDf->phiInstructionIndexes.push_back(0);
					//since we have actually just added a variable to the new node
					// we need to update our block
					workList.push(blockDf);
				}
			}
		}
	}
	// renaming stage
	Rename(startBlock);

}
void CFG::BuildDF()
{
	for (Block* b : tpgSort) {
		if (b->parents.size() >= 2) {
			for (Block* p : b->parents) {
				Block* runner = p;
				while (runner != b->idom) {
					runner->df.insert(b);
					runner = runner->idom;
				}
			}
		}
	}
}

void CFG::TopSort()
{
	Bfs(startBlock, [&](Block* block) {tpgSort.push_back(block); });
}

void CFG::Bfs(Block* start, std::function<void (Block*)> action)
{
	std::queue<Block*> blockQueue;
	std::unordered_set<Block*> visitedBlocks;

	blockQueue.push(startBlock);
	visitedBlocks.insert(startBlock);

	while (!blockQueue.empty())
	{
		auto blockToPrint = blockQueue.front();
		blockQueue.pop();
		
		action(blockToPrint);

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

void CFG::Debug()
{
	// just current block for now
	// once we add control flow we should traverse blocks
	assert(startBlock != nullptr);

	std::queue<Block*> blockQueue;
	std::unordered_set<Block*> visitedBlocks;

	Bfs(startBlock, PrintBlock);
}
void CFG::CreateVariable(const Node* tree)
{
	auto expr = tree->As<Expression>();
	// left is variable
	auto left = expr->left->As<Expression>();
	auto name = left->value.AsString();
	int version = 0;
	// get a version of a variable
	// local variable
	if (left->depth > 0)
	{
		//version = GetVersion(localVariables, *name);
		variableCounterLocal[name->GetRaw()] = 0;
		localAssigned[*name].push_back(currentBlock);
	}
	// global variable
	else
	{
		//version = GetVersion(globalVariables, *name);
		globalAssigned[*name].push_back(currentBlock);
	}
	//assert(version != -1);
	std::stringstream ss;
	ss << std::string(name->GetRaw());
	//ss << "_";
	//ss << std::to_string(version);
	Operand resOp{ ss.str() ,false,version };
	auto rightOp = ConvertExpressionAST(expr->right.get());

	auto instruction = Instruction{ TokenType::EQUAL,{},rightOp,resOp };
	currentBlock->instructions.push_back(instruction);

}

Operand CFG::BinaryInstr(const Expression* expr, TokenType type)
{
	auto left = ConvertExpressionAST(expr->left.get());
	auto right = ConvertExpressionAST(expr->right.get());
	//auto version = GetTempVersion();
	auto version = 0;
	std::stringstream resultName;
	resultName << "t";
	//resultName << version;
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

void CFG::FindIDoms()
{
	// we could use instead of this post order traversal numbers
	// but that should do?
	for (Block* b : tpgSort) {

		// Get the strict dominators of b
		// all dominators except of oneself
		std::set<Block*> strict_dom = b->dom;
		strict_dom.erase(b);


		for (Block* d : strict_dom) {
			bool isImmediate = true;

			// Check if d is dominated by any other dominator in strict_dom
			for (Block* other_d : strict_dom) {
				if (other_d == d) continue;
				if (d->dom.find(other_d) != d->dom.end()) {
					// d is dominated by other_d, so it's not the immediate dominator
					// if we follow the definition
					isImmediate = false;
					break;
				}
			}

			if (isImmediate) {
				b->idom = d;
				break;  
			}
		}
	}
}

void CFG::FindDoms()
{
	bool isChanged = true;


	// Initialize dominator sets
	for (auto block : tpgSort) {
		block->dom.insert(block);  // A node dominates itself
		if (block != startBlock) {
			for (auto dominator : tpgSort) {
				block->dom.insert(dominator);  // Initially, all nodes can dominate any other
			}
		}
	}

	// Iterate until the dominator sets stabilize
	while (isChanged) {
		isChanged = false;

		for (auto block : tpgSort) {
			if (block == startBlock) continue;  // Root is its own dominator

			std::set<Block*> newDom;

			for (auto parent : block->parents)
			{
				if (newDom.empty()) {
					newDom = parent->dom;  // First predecessor
				}
				else {
					// Compute the intersection (LCA?????) of dominator sets
					std::set<Block*> temp;
					set_intersection(newDom.begin(), newDom.end(),
						parent->dom.begin(), parent->dom.end(),
						inserter(temp, temp.begin()));
					newDom = temp;
				}
			}
			newDom.insert(block);

			// If the dominator set changed, update it
			if (block->dom != newDom) {
				block->dom = newDom;
				isChanged = true;
			}
		}
	}
}

// the simplest case is for acyclic graph
// dom (u) = lowest common ancestor (dom v), where v is a incoming neighbour

void CFG::BuildDominatorTree()
{
	FindDoms();
	FindIDoms();

	for (auto block : tpgSort) {
		if (block->idom != nullptr) {
			block->idom->dominatorChildren.push_back(block);
		}
	}


}

void CFG::ConvertAST(const Node* tree)
{
	auto expr = tree->As<Expression>();
	auto type = tree->type;

	startBlock = currentBlock = CreateBlock("[block_0]", {});
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
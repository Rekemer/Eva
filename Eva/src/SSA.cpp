#include "SSA.h"
#include "AST.h"
#include "VirtualMachine.h"
#include "Tokens.h"
#include <cassert>
#include <queue>
#include <unordered_set>
#include <format>



//https://www.cs.cornell.edu/courses/cs6120/2023fa/lesson/6/

//An operation is critical if it sets return values for the procedure,
//it is an input/output statement, 
//or it affects the value in a storage location that may be accessible 
//from outside the current procedure

void PrintValue(std::ostream& os, const ValueContainer& v, int version, bool IsConstant)
{
	if (IsConstant || version == LABEL_VERSION)
	{
		os << v;
		return;
	}
	os << v << "_" << version;
}
std::ostream& operator<<(std::ostream& os, const Instruction& v)
{	
	auto& resValue = v.result.value;
	auto& leftValue = v.operLeft.value;
	auto& rightValue = v.operRight.value;
	os << resValue << "_" <<  v.result.version << " = ";
	if (leftValue.type != ValueType::NIL ) PrintValue(os, leftValue, v.operLeft.version, v.operLeft.isConstant);
	os << " " << tokenToString(v.instrType) << " ";
	if (v.instrType == TokenType::PHI || v.instrType == TokenType::BRANCH)
	{
		os << "( ";
		for (auto& param : v.variables)
		{
			os << param.value << "_" << param.version << ", ";
		}
		os << ") ";
	}
	if (rightValue.type != ValueType::NIL ) PrintValue(os, rightValue, v.operRight.version, v.operRight.isConstant);
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
		auto type = vm->GetGlobalsType().Get(call->name)->value.type;
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
	case TokenType::DECLARE:
	case TokenType::EQUAL:
	case TokenType::PLUS_EQUAL:
	case TokenType::MINUS_EQUAL:
	case TokenType::SLASH_EQUAL:
	case TokenType::STAR_EQUAL:
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
int GetVersion(std::unordered_map<std::string, int>& map, std::string& name)
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

Operand NullOperand()
{
	return Operand{ ValueContainer {std::string{"null"}},false,NOT_INIT_VERSION };
};


Block* CFG::CreateBranchBlock(Block* parentBlock,Instruction& branch, Node* block, const std::string& BlockName, const std::string& mergeName)
{
	auto then = CreateBlock(BlockName, { parentBlock });

	parentBlock->blocks.push_back(then);
	currentBlock = then;

	if (block != nullptr)
	{
		ConvertStatementAST(block);
	}

	
	ValueContainer name = ValueContainer{ mergeName };
	Operand mergeOperand = { name, false, LABEL_VERSION };
	currentBlock->instructions.
		push_back(Instruction{ TokenType::JUMP,{},mergeOperand, NullOperand() });

	
	branch.targets.push_back(then);
	return then;
}



void PrintBlock(Block* block)
{
	std::cout  << std::endl << block->name << std::endl;
	//std::cout << "------------" << std::endl;
	for (auto& instr : block->instructions)
	{
		std::cout << instr;
	}

	std::cout << "[dominator children]\n[ ";

	for (auto dominator : block->dominatorChildren)
	{
		if (dominator != block)
			std::cout << dominator->name << " ";
	}
	std::cout << "]" << std::endl;

	std::cout << "[children]\n[ ";

	for (auto dominator : block->blocks)
	{
		if (dominator != block)
			std::cout << dominator->name << " ";
	}
	std::cout << "]" << std::endl;

	std::cout << "[parents]\n[ ";

	for (auto dominator : block->parents)
	{
		if (dominator != block)
			std::cout << dominator->name << " ";
	}
	std::cout << "]" << std::endl;

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
int CFG::NewName(const std::string& name)
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
		phi.result.version = NewName(phi.result.value.AsString());
	}

	auto& types = vm->GetGlobalsType();
	// update each operation
	for (auto& instr : b->instructions)
	{
		if (instr.instrType == TokenType::PHI || instr.instrType == TokenType::JUMP ||
			instr.instrType == TokenType::BLOCK) continue;
		if (instr.instrType == TokenType::BRANCH && instr.operRight.IsVariable())
		{
			instr.operRight.version = variableStack[instr.operRight.value.AsString()].top();
			continue;
		}
		
		if (instr.operRight.IsVariable())
		{
			instr.operRight.version = variableStack[instr.operRight.value.AsString()].top();
		}
		if (!instr.IsUnary())
		{
			if (instr.operLeft.IsVariable())
			{
				instr.operLeft.version = variableStack[instr.operLeft.value.AsString()].top();
			}
		}
		
		// versioning of temporary varoables is not handled by stack
		if(!instr.result.IsTemp()) instr.result.version = NewName(instr.result.value.AsString());
		if (instr.instrType != TokenType::PRINT)
		{
			std::string name;
			if (instr.result.IsTemp())
			{
				name = instr.result.value.AsString() + "_" + std::to_string(instr.result.version);
			}
			else name = instr.result.value.AsString();
		}
	}


	auto updatePhiParam = [](Block*b, Block* child,
		std::unordered_map<std::string, std::stack<int>>& variableStack)
		{
			for (auto index : child->phiInstructionIndexes)
			{
				auto& phi = child->instructions[index];
				auto& varName = phi.result;
				if (variableStack[varName.value.AsString()].size() > 0)
				{
					// we cannot simply iterate over stack
					// because the parents of merge node 
					// have not been proccesed yet
					auto parent = std::find(child->parents.begin(), child->parents.end(), b);
					if (parent == child->parents.end())
					{
						assert(false && "parent is not found!? ");
					}
					auto index = std::distance(child->parents.begin(), parent);
					phi.variables[index].version = variableStack[varName.value.AsString()].top();
					phi.variables[index].value = varName.value;
				}

			}
		};

	// update phi parametrs
	for (auto child : b->blocks)
	{

		updatePhiParam(b, child, variableStack);
		//for (auto index : child->phiInstructionIndexes)
		//{
		//	auto& phi = child->instructions[index];
		//	auto& varName = phi.result;
		//	if (variableStack[varName.value.AsString()].size() > 0)
		//	{
		//		// we cannot simply iterate over stack
		//		// because the parents of merge node 
		//		// have not been proccesed yet
		//		auto parent = std::find(child->parents.begin(), child->parents.end(), b);
		//		if (parent == child->parents.end())
		//		{
		//			assert(false && "parent is not found!? ");
		//		}
		//		auto index = std::distance(child->parents.begin(),parent);
		//		phi.variables[index].version = variableStack[varName.value.AsString()].top();
		//		phi.variables[index].value= varName.value;
		//	}
		//
		//}
	}

	if (b->merge)
	{
		updatePhiParam(b, b->merge, variableStack);
	}

	for (auto s : b->dominatorChildren)
	{
		Rename(s);
	}

	// Pop variable versions for variables defined in this block

	for (auto it = b->instructions.rbegin(); it != b->instructions.rend(); ++it)
	{
		if (it->variables.size() > 0) continue;
		if (it->instrType == TokenType::JUMP || it->instrType == TokenType::BRANCH) continue;
		if (it->result.IsVariable())
		{
			auto varName = it->result.value.AsString();
			variableStack[varName].pop();
		}
	}


	for (auto index : b->phiInstructionIndexes)
	{
		auto& phi = b->instructions[index];
		variableStack[phi.result.value.AsString()].pop();
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
					auto name = v.first;
										// Operand{name} is result  -  x = phi x x x x 
					auto instr = Instruction{ TokenType::PHI ,{},{},Operand{ValueContainer{name},false,NOT_INIT_VERSION} };
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
			auto merge = block->merge;
			if (merge != nullptr && visitedBlocks.find(merge) == visitedBlocks.end())
			{
				blockQueue.push(merge);
				visitedBlocks.insert(merge);
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


void  CFG::InitLocal(Operand& op,const std::string& name)
{
	variableCounterLocal[name] = 0;
	localAssigned[name].push_back(currentBlock);
	auto [isExist, index, depth] = currentScope->IsLocalExist(name, currentScope->depth);
	assert(isExist);
	op.index = index;
	op.depth = depth;
	op.type = currentScope->GetType(name);
}
void CFG::InitGlobal(Operand& op,const std::string& name)
{
	localAssigned[name].push_back(currentBlock);
	op.type = vm->GetGlobalType(name);
	op.depth = 0;
}
Operand CFG::InitVariable(const std::string& name, int depth)
{
	Operand resOp{ ValueContainer{name},false,NOT_INIT_VERSION };
	if (depth > 0)
	{
		InitLocal(resOp, name);
	}
	// global variable
	else
	{
		InitGlobal(resOp, name);
	}
	return resOp;
}
void CFG::CreateVariable(const Node* tree, TokenType type)
{
	auto expr = tree->As<Expression>();
	// left is variable
	auto left = expr->left->As<Expression>();
	
	ValueType typeV = ValueType::NIL;
	
	auto resOp = InitVariable(left->value.AsString(), left->depth);
	
	auto rightOp = ConvertExpressionAST(expr->right.get());

	auto instruction = Instruction{ type,{},rightOp,resOp };
	
	instruction.returnType = rightOp.type;
	currentBlock->instructions.push_back(instruction);

}

void CFG::CreateVariableFrom(const Node* tree, const Operand& rightOp)
{
	auto expr = tree->As<Expression>();
	// left is variable
	auto left = expr->left->As<Expression>();
	auto name = left->value.AsString();
	Operand resOp{ ValueContainer{name},false,-1 };
	// local variable
	if (left->depth > 0)
	{
		
		InitLocal(resOp, name);
	}
		// global variable
	else
	{
		InitGlobal(resOp, name);
	}

	auto instruction = Instruction{ TokenType::EQUAL,{},rightOp,resOp };
	currentBlock->instructions.push_back(instruction);

}

Operand CFG::CreateTemp()
{
	Operand temp{ ValueContainer{ "t"} ,false,tempVersion++};
	temp.isTemp = true;
	if(currentScope == nullptr)
	temp.depth = 0;
	else temp.depth = currentScope->depth;
	return temp;
}

void GiveType(Instruction& instr, Operand& resOp, ValueType type)
{
	instr.returnType = type;
	resOp.type = type;
}

// create temp because it is separate operation which is stored
// in temp variable
Operand CFG::UnaryInstr(const Expression* expr, TokenType type)
{
	auto left = ConvertExpressionAST(expr->left.get());
	auto res = CreateTemp();
	Instruction instr{ type,{},left,res };
	GiveType(instr, res, expr->value.type);
	currentBlock->instructions.push_back(instr);
	return res;
}
Operand CFG::BinaryInstr(const Expression* expr, TokenType type)
{
	auto left = ConvertExpressionAST(expr->left.get());
	auto right = ConvertExpressionAST(expr->right.get());
	auto res = CreateTemp();
	Instruction instr{ type,left,right,res };
	GiveType(instr, res, expr->value.type);
	currentBlock->instructions.push_back(instr);
	return res;
}

void CFG::ConvertStatementAST(const Node* tree)
{
	auto GetOpFromComplexAssignment = [](TokenType assignment)
		{
			switch (assignment)
			{
			case TokenType::MINUS_EQUAL:
				return TokenType::MINUS;
			case TokenType::PLUS_EQUAL:
				return TokenType::PLUS;
			case TokenType::SLASH_EQUAL:
				return TokenType::SLASH;
			case TokenType::STAR_EQUAL:
				return TokenType::STAR;
			default:
				assert(false);
				break;
			}
		};
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
		CreateVariable(tree,type);
		break;
	}
	case TokenType::MINUS_EQUAL:
	case TokenType::PLUS_EQUAL:
	case TokenType::SLASH_EQUAL:
	case TokenType::STAR_EQUAL:
	{
		auto rightOperand = BinaryInstr(tree->As<Expression>(), GetOpFromComplexAssignment(type));
		CreateVariableFrom(tree, rightOperand);
		break;
	}
	case TokenType::CONTINUE:
	case TokenType::BREAK:
	case TokenType::FOR:
	case TokenType::FUN:
	case TokenType::IF:
	{
		auto flows = expr->right.get();
		auto thenBranch = flows->As<Expression>()->right.get();
		// we need to convert the condition
		auto condition = expr->left.get();
		auto parentBlock = currentBlock;
		//Instruction branch;
		Instruction branch = Instruction{ TokenType::BRANCH,{},{},NullOperand() };

		branch.operRight = ConvertExpressionAST(condition);

		auto thenBlockName = std::format("[then_{}]", std::to_string(Block::counterThen++));
		std::vector<Block*> mergeParents;
		std::vector<Block*> elifBlocks;
		auto mergeName = std::format("[merge_{}]", std::to_string(Block::counterMerge++));
		//// then
		auto then = CreateBranchBlock(parentBlock, branch, thenBranch, thenBlockName, mergeName);
		parentBlock->instructions.push_back(branch);
		mergeParents.push_back(currentBlock);

		bool isElseCase = flows->As<Expression>()->left != nullptr;

		Block* prevConditionBlock = parentBlock;
		if (isElseCase)
		{
			// handle elif cases
			bool isElif = flows->As<Expression>()->left != nullptr ? flows->As<Expression>()->left.get()->type == TokenType::IF : false;
			if (isElif)
			{
				std::vector<Instruction> branchesElif;
				auto elifNode = flows->As<Expression>()->left.get();
				bool firstElif = false;
				while (elifNode != nullptr && elifNode->type != TokenType::BLOCK && elifNode->type == TokenType::IF)
				{
					currentBlock = parentBlock;
					Instruction branchElif = Instruction{ TokenType::BRANCH_ELIF,{},{},NullOperand() };
					auto elifBlockName = std::format("[elif_{}]", std::to_string(Block::counterElif));
					auto conditionNode = elifNode->As<Expression>()->left.get();

					auto conditionName = std::format("[elif_condition_{}]", std::to_string(Block::counterElif++));
					auto conditionBlock = CreateBlock(conditionName, { currentBlock });
					currentBlock->blocks.push_back(conditionBlock);
					if (prevConditionBlock)
					{
						prevConditionBlock->instructions.back().targets.push_back(conditionBlock);
					}
					currentBlock = conditionBlock;
					branchElif.operRight =  ConvertExpressionAST(conditionNode);

					//if (!firstElif)
					//{
					//	branch.targets.push_back(conditionBlock);
					//	parentBlock->instructions.push_back(branch);
					//	firstElif = true;
					//}

					// body 
					auto elifFlows = elifNode->As<Expression>()->right.get();
					auto block = elifFlows->As<Expression>()->right.get();
					auto body = CreateBranchBlock(conditionBlock, branchElif, block,elifBlockName, mergeName);

					conditionBlock->instructions.push_back(branchElif);


				

					mergeParents.push_back(currentBlock);
					elifBlocks.push_back(body);
					branchesElif.push_back(branchElif);

					prevConditionBlock= conditionBlock;
					elifNode = elifNode->As<Expression>()->right->As<Expression>()->left.get();
				}
				flows = elifNode;
			}
			else
			{
				flows = flows->As<Expression>()->left.get();
			}
		}
		else
		{
			flows = nullptr;
		}
		if(flows != nullptr) assert(flows->type == TokenType::BLOCK);
		
		auto elseBranch = flows;
		auto elseBlockName = std::format("[else_{}]", std::to_string(Block::counterElse++));
		auto els = CreateBranchBlock(parentBlock, branch, elseBranch, elseBlockName, mergeName);

		if (prevConditionBlock)
		{
			prevConditionBlock->instructions.back().targets.push_back(els);
		}

		mergeParents.push_back(currentBlock);







		auto merge = CreateBlock(mergeName, { });

		for (auto parent : mergeParents)
		{

			parent->merge = merge;
			merge->parents.push_back(parent);
		}

		for (auto& elif : elifBlocks)
		{
			elif->merge = merge;
			//elif->blocks.push_back(merge);
		}
		currentBlock = merge;


		break;
	}
	case TokenType::PRINT:
	{
		auto value = ConvertExpressionAST(expr->left.get());
		Instruction instr{ type,{},value, CreateTemp()};
		currentBlock->instructions.push_back(instr);
		break;
	}
	case TokenType::RETURN:
	case TokenType::WHILE:
	case TokenType::BLOCK:
	{
		Block* block = currentBlock;
		auto scope = tree->As<Scope>();
		currentScope = scope;

		for (auto& statement : scope->expressions)
		{
			ConvertStatementAST(statement.get());
		}
		if (currentScope->prevScope)
		{
			currentScope = currentScope->prevScope;
		}
		// to know how many times we need to pop
		Instruction instr{ TokenType::BLOCK,{},Operand{scope->popAmount,false,NOT_INIT_VERSION},{} };
		currentBlock->instructions.push_back(instr);
		break;
	}
	case TokenType::DEDUCE:
		assert(false && "not all types are deduced");
	default:
		//assert(false && "we should not be here");
		ConvertExpressionAST(tree);
		break;
	}
}

Operand CFG::ConvertExpressionAST(const Node* tree)
{
	auto type = tree->type;
	auto expr = tree->As<Expression>();
	
	switch (type)
	{

	case TokenType::IDENTIFIER:
	{
		return InitVariable(expr->value.AsString(),expr->depth);
		break;
	}
	case TokenType::PLUS_PLUS:
	case TokenType::MINUS_MINUS:
	case TokenType::PLUS:
	case TokenType::STAR:
	case TokenType::MINUS:
	case TokenType::SLASH:
	case TokenType::PERCENT:
	{
		Operand res;
		if (type == TokenType::MINUS  && expr->right == nullptr)
		{
			res = UnaryInstr(expr, type);
		}
		else if (type == TokenType::PLUS_PLUS || type == TokenType::MINUS_MINUS)
		{
			auto var = ConvertExpressionAST(expr->left.get());
			Instruction action{ type,{},{},var };
			GiveType(action, action.result, expr->value.type);
			currentBlock->instructions.push_back(action);
			return var;

		}
		else  res = BinaryInstr(expr, type);
		return res;
		break;
	}
	case TokenType::INT_LITERAL:
	{
		auto number = expr->value.As<int>();
		Operand op{ number,true,0 };
		op.type = ValueType::INT;
		return op;
		break;
	}
	case TokenType::FLOAT_LITERAL:
	{
		auto number = expr->value.As<float>();
		Operand op{ number,true,0 };
		op.type = ValueType::FLOAT;
		return op;
		break;
	}
	case TokenType::STRING_LITERAL:
	{
		auto number = expr->value.As<std::string>();
		Operand op{ number,true,0 };
		op.type = ValueType::STRING;
		return op;
		break;
	}
	case TokenType::TRUE:
	case TokenType::FALSE:
	{
		auto value = expr->value.As<bool>();
		Operand op{ value,true,0 };
		op.type = ValueType::BOOL;
		return op;
		break;
	}
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
	case TokenType::EQUAL_EQUAL:
	case TokenType::BANG_EQUAL:
	case TokenType::LESS_EQUAL:
	case TokenType::LESS:
	case TokenType::AND:
	case TokenType::OR:
	{
		return BinaryInstr(tree->As<Expression>(), type);
		break;
	}
	case TokenType::BANG:
	{
		auto res = UnaryInstr(expr, type);
		return res;
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
				if (other_d->dom.find(d) != other_d->dom.end()) {
					// If d is dominated by another dominator, it's not the immediate dominator
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
	
	
	
	Block::counterStraight++;
	if (IsStatement(tree))
	{
		ConvertStatementAST(tree);
	}
	else
	{
		ConvertExpressionAST(tree);
		//assert(false && "We are not supposed to have non statement at this level?");
	}
	
	
}
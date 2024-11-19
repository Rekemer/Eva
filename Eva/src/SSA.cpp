#include "SSA.h"
#include "AST.h"
#include "Nodes.h"
#include "VirtualMachine.h"
#include "Tokens.h"
#include <cassert>
#include <unordered_set>
#include <format>



//https://www.cs.cornell.edu/courses/cs6120/2023fa/lesson/6/

//An operation is critical if it sets return values for the procedure,
//it is an input/output statement, 
//or it affects the value in a storage location that may be accessible 
//from outside the current procedure
void PrintBlock(Block* block);

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
	if (v.instrType == TokenType::VAR) return os;
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
	if (v.instrType == TokenType::LEFT_PAREN)
	{
		os << "\n(arg block)\n";
		PrintBlock(v.argBlock);
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
		//auto call = node->As<Call>();
		//auto type = vm->GetGlobalType(call->name);
		//if (type == ValueType::NIL) 
		//{
			return false;
		//}
		//return false;
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
Block* CFG::CreateBlock(const std::string& currentFunction ,const std::string& name, std::vector<Block*> parents)
{
	Block* block = &graph[name];
	block->name = name;
	block->parents = parents;
	if (functionCFG[currentFunction].start == nullptr)
	{
		functionCFG[currentFunction].start = functionCFG[currentFunction].current = block;
	}
	else
	{
		functionCFG[currentFunction].current = block;
	}
	return functionCFG[currentFunction].current;
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
	auto then = CreateBlock(currentFunc,BlockName, { parentBlock });

	parentBlock->blocks.push_back(then);
	currentBlock = then;

	if (block != nullptr)
	{
		ConvertStatementAST(block);
	}

	
	auto jump = Instruction{ TokenType::JUMP_BRANCH,{}, {}, NullOperand() };
	currentBlock->instructions.
		push_back(jump);

	
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
	std::cout << std::endl;
	//std::cout << "[dominator children]\n[ ";
	//
	//for (auto dominator : block->dominatorChildren)
	//{
	//	if (dominator != block)
	//		std::cout << dominator->name << " ";
	//}
	//std::cout << "]" << std::endl;
	//
	//std::cout << "[children]\n[ ";
	//
	//for (auto dominator : block->blocks)
	//{
	//	if (dominator != block)
	//		std::cout << dominator->name << " ";
	//}
	//std::cout << "]" << std::endl;
	//
	//std::cout << "[parents]\n[ ";
	//
	//for (auto dominator : block->parents)
	//{
	//	if (dominator != block)
	//		std::cout << dominator->name << " ";
	//}
	//std::cout << "]" << std::endl;
	//
	//std::cout << "[dominators]\n[ ";
	//
	//for (auto dominator : block->dom)
	//{
	//	if (dominator != block)
	//	std::cout << dominator->name << " ";
	//}
	//std::cout << "]" << std::endl;
	//
	//if (block->idom)
	//{
	//	std::cout << "[immediate dominator] " << block->idom->name << std::endl;
	//}
	//
	//if (block->df.size() > 0)
	//{
	//	std::cout << "[dominance frontier] ";
	//	for (auto b : block->df)
	//	{
	//		std::cout << b->name;
	//	}
	//	std::cout << std::endl;
	//}
	//
	//if (block->merge)
	//{
	//	std::cout << "[merge]\n[ ";
	//	std::cout << block->merge->name;
	//	std::cout << "]" << std::endl;
	//}
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
		AddDef(phi.result.depth, phi.result.value.AsString(), index,b);
	}

	auto& types = vm->GetGlobalsType();
	// update each operation
	auto index = 0;
	for (int i = 0; i <  b->instructions.size(); i++)
	{
		auto& instr = b->instructions[i];
		if (instr.instrType == TokenType::PHI || instr.instrType == TokenType::JUMP ||
			instr.instrType == TokenType::BLOCK || instr.instrType == TokenType::JUMP_BACK || instr.instrType == TokenType::JUMP_FOR
			|| instr.instrType == TokenType::LEFT_PAREN || instr.instrType == TokenType::CALL|| instr.instrType == TokenType::FUN) continue;
		if (instr.instrType == TokenType::BRANCH && instr.operRight.IsVariable())
		{
			instr.operRight.version = variableStack[instr.operRight.value.AsString()].top();
			AddUse(instr.operRight.depth, instr.operRight.GetVariableVerName(),i,b);
			continue;
		}
		
		if (instr.operRight.IsVariable())
		{
			instr.operRight.version = variableStack[instr.operRight.value.AsString()].top();
			AddUse(instr.operRight.depth, instr.operRight.GetVariableVerName(), i,b);
		}
		if (!instr.IsUnary())
		{
			if (instr.operLeft.IsVariable())
			{
				instr.operLeft.version = variableStack[instr.operLeft.value.AsString()].top();
				AddUse(instr.operLeft.depth, instr.operLeft.GetVariableVerName(), i,b);
			}
		}
		
		// versioning of temporary varoables is not handled by stack
		if (!instr.result.IsTemp())
		{
			instr.result.version = NewName(instr.result.value.AsString());
		}
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
						//assert(false && "parent is not found!? ");
					}
					else
					{
						auto index = std::distance(child->parents.begin(), parent);
						phi.variables[index].version = variableStack[varName.value.AsString()].top();
						phi.variables[index].value = varName.value;
						phi.targets.push_back(*parent);
					}
				}

			}
		};

	// update phi parametrs
	for (auto child : b->blocks)
	{

		updatePhiParam(b, child, variableStack);
		
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
		if (it->instrType == TokenType::JUMP_FOR || it->instrType == TokenType::JUMP || it->instrType == TokenType::BRANCH) continue;
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

Instruction CFG::CreatePhi(const std::string& name)
{	
	auto instr = Instruction{ TokenType::PHI ,{},{},Operand{ValueContainer{name},false,NOT_INIT_VERSION} };
	return instr;
}
void MakeCritical(Instruction& instr)
{
	instr.isCritical = true;
}
void CFG::InsertPhi()
{
	// placing stage
	for (auto& v : localAssigned)
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
					// Check if varName is assigned along multiple paths to blockDf
					int defsInPredecessors = 0;
					for (auto pred : blockDf->parents)
					{

						auto iter = std::find(localAssigned[v.first].begin(), localAssigned[v.first].end(), pred);
						if (iter != localAssigned[v.first].end())
						{
							defsInPredecessors++;
						}
						else
						{
							//defsInPredecessors--;
							if (pred->parents.size() == 0)
							{
								defsInPredecessors--;
								continue;
							}
							auto iter = std::find(localAssigned[v.first].begin(), localAssigned[v.first].end(),
								pred->parents[0]);
							if (iter == localAssigned[v.first].end())
							{
								defsInPredecessors--;
							}
							else 
								defsInPredecessors++;
						}
					}
					if (defsInPredecessors < 1) continue;
					
					hasAlready.insert(blockDf);
					auto name = v.first;
					
					
					// Operand{name} is result  -  x = phi x x x x 
					auto instr = CreatePhi(name);
					auto potentialChanges = blockDf->parents.size();
					instr.variables.resize(potentialChanges);
					blockDf->instructions.insert(blockDf->instructions.begin(), instr);

					blockDf->offsetPhi++;
					for (auto& [k,v]: blockDf->defs)
					{
						for (auto& index : v)
						{
							index += (blockDf->offsetPhi );
						}
					}
					blockDf->offsetPhi = 0;
					
					if (blockDf->phiInstructionIndexes.empty())
					{
						blockDf->phiInstructionIndexes.push_back(0);
					}
					else
					{
						blockDf->phiInstructionIndexes.push_back(blockDf->phiInstructionIndexes.back() + 1);

					}
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
void MarkInstruction(Instruction& instr, Block* b)
{
	instr.isMarked = true;
}
bool isBranchType(TokenType instrType)
{
	return static_cast<int>(instrType) >= static_cast<int>(TokenType::JUMP_BRANCH) &&
		static_cast<int>(instrType) <= static_cast<int>(TokenType::BRANCH_FOR);
}

auto findDef = [](Block* block, int depth, const std::string& name)-> std::vector<int>
{
	auto b = block;
	std::unordered_map<Block*, std::vector<int>> defs;
	auto iter = b->defs.find(std::pair{ depth,name });
	return  iter != b->defs.end() ? iter->second : std::vector<int>{};
};

void  CFG::MarkOperand(Block* block, Operand& oper, std::queue<std::pair<Block*, Instruction*>>& workList)
	{
		if (oper.IsVariable() or oper.IsTemp())
		{
			auto name = oper.IsTemp() ? oper.GetTempName() : oper.value.AsString();
			std::queue<Block*> defsBlocks;
			defsBlocks.push(block);
			if (oper.depth > 0)
			{
				while (!defsBlocks.empty())
				{
					auto b = defsBlocks.front();
					defsBlocks.pop();
					auto defs = findDef(b, oper.depth, name);
					for (auto i : defs)
					{
						if (!b->instructions[i].isMarked)
						{
							MarkInstruction(b->instructions[i], b);
							workList.push({ b,&b->instructions[i] });
						}
					}
					for (auto p : b->parents)
					{
						defsBlocks.push(p);
					}
				}

			}
			else
			{
				// not optimal
				for (auto [k, v] : globalDefs)
				{
					if (k.second == name)
					{
						auto b = k.first;
						for (auto i : v)
						{
							if (!b->instructions[i].isMarked)
							{
								MarkInstruction(b->instructions[i], b);
								workList.push({ b,&b->instructions[i] });
							}
						}
					}
				}
			}
			for (auto index : block->phiInstructionIndexes)
			{
				auto& instr = block->instructions[index];

				if (!instr.isMarked && instr.result.value.AsString() == name)
				{
					MarkInstruction(instr, block);
					workList.push({ block,&instr });

				}
			}
		}
	}

void CFG::Mark (Block* b, std::queue<std::pair<Block*, Instruction*>>& workList)
	{
		for (auto& inst : b->instructions)
		{
			//inst.isMarked = false;
			if (inst.isCritical || b->markAll)
			{
				MarkInstruction(inst, b);
				workList.push({ b,&inst });
			}
		}
		std::vector<Instruction*> funcCalls;
		while (!workList.empty())
		{
			auto info = workList.front();
			workList.pop();
			auto instr = info.second;
			if (instr->instrType != TokenType::FUN)
			{
				MarkOperand(info.first, instr->operRight, workList);
				MarkOperand(info.first, instr->operLeft, workList);
				MarkOperand(info.first, instr->result, workList);
				if (instr->instrType == TokenType::LEFT_PAREN)
				{
					assert(instr->argBlock != nullptr);
					funcCalls.push_back(instr);

				}
			}
			//auto instr = &block->instructions[i];
			if (instr->instrType == TokenType::PHI)
			{
				assert(instr->targets.size() == instr->variables.size());
				for (int i = 0; i < instr->targets.size(); i++)
				{
					instr->variables[i].depth = 1;
					MarkOperand(instr->targets[i], instr->variables[i], workList);
				}
			}
			for (auto block : info.first->rdf)
			{
				auto& branch = block->instructions.back().instrType
					== TokenType::BLOCK ? *(block->instructions.end() - 2) :
					block->instructions.back();
				if (!branch.isMarked)
				{
					MarkInstruction(branch, b);
					workList.push({ block,&branch });
				}
			}
		}
		for (auto f : funcCalls)
		{
			Mark(f->argBlock, workList);
		}
		for (auto child : b->blocks)
		{
			Mark(child, workList);
		}
		if (b->merge != nullptr)
		{
			Mark(b->merge, workList);
		}
		return;
}

std::unordered_map<std::pair<int, std::string>, int, pair_hash> removedLocal;
std::unordered_map<int, int> removedLocalTotal;
std::unordered_map<int, int> declaredLocal;



std::unordered_map<std::string , bool> updateDefsUses;
void AdjustOperandIndex(Operand& operand, 
	const std::unordered_map<std::pair<int, std::string>, int, pair_hash>& removedLocal,
	Block* b)
{
	auto varName = operand.value.AsString();
	// Retrieve the total removed count based on depth and value
	auto key = std::pair{ operand.depth,varName };
	int total = removedLocal.at(key);
	//if (!updateDefsUses[varName] && b->updateIndex)
	//{
	//	auto isDef = b->defs.find(key) != b->defs.end();
	//	if (isDef)
	//	{
	//		for (auto& i : b->defs.at(key))
	//		{
	//			i -= total;
	//			assert(i >= 0);
	//		}
	//	}
	//	for (auto& i : b->uses.at(varName))
	//	{
	//		i -= total;
	//		assert(i >= 0);
	//	}
	//	updateDefsUses[varName] = true;
	//}
	
	// Adjust the index by subtracting the total removed count
	operand.index -= total;

	// Ensure the index remains non-negative
	assert(operand.index >= 0);
}

// sweep 
void CFG::Sweep(Block* block)
	{
		block->isSweeped = true;
		auto currentDepth = 0;
		auto& instructions = block->instructions;
		for (auto it = instructions.begin(); it != instructions.end(); )
		{
			if (it->instrType == TokenType::FUN)
			{
				it++;
				continue;

			}
			auto isJump = (int)it->instrType >= (int)TokenType::JUMP and
				(int)it->instrType <= (int)TokenType::JUMP_WHILE;
			if (!it->isMarked)
			{

				if (!isJump)
				{
					if (it->instrType == TokenType::DECLARE && it->result.IsVariable())
					{
						removedLocalTotal[it->result.depth]++;

					}
					if (!isBranchType(it->instrType))
					{
						it = instructions.erase(it);
						block->updateIndex = true;
					}
					else
					it++;
				}
				else
				{

					it++;
				}
			}
			else
			{
				if (it->operRight.depth == 0 )
				{
					it++;
					continue;
				}
				if (isJump)
				{
					Sweep(it->targets[0]);
				}

				if (it->instrType == TokenType::LEFT_PAREN)
				{
					Sweep(it->argBlock);
				}
				if (it->instrType == TokenType::BRANCH)
				{
					for (auto target : it->targets)
					{
						Sweep(target);
					}

				}
				// declare a variable
				if (it->instrType == TokenType::DECLARE && it->result.IsVariable() && it->result.depth > 0)
				{
					//set(declaredLocal, 0);
					int total = 0;
					for (const auto& pair : removedLocalTotal) {
						if (pair.first <= it->result.depth)
							total += pair.second;
					}

					removedLocal[{it->result.depth, it->result.value.AsString()}] =
						total;

				}
				if (it->instrType == TokenType::BLOCK)
				{
					auto flowCommand = (it + 1);
					if (flowCommand != instructions.end() &&
						(flowCommand->instrType == TokenType::CONTINUE || flowCommand->instrType == TokenType::BREAK))
					{
						auto total = 0;
						auto loopDepth = flowCommand->operRight.value.As<int>();
						for (auto [k, v] : removedLocalTotal)
						{
							if (k >= loopDepth)
							{
								total+=removedLocalTotal.at(k);
								//removedLocalTotal.at(k) = 0;
							}
						}
						it->operRight.value.AsRef<int>() -= total;
					}
					else
					{
						auto iter = removedLocalTotal.find(it->operRight.depth);
						if (iter != removedLocalTotal.end())
						{
							it->operRight.value.AsRef<int>() -= removedLocalTotal.at(it->operRight.depth);
							removedLocalTotal.at(it->operRight.depth) = 0;
						}
					}
				}
				else if (it->instrType == TokenType::RETURN)
				{
					auto iter = removedLocalTotal.find(it->operRight.depth);
					if (iter != removedLocalTotal.end())
					{
						it->operLeft.value.AsRef<int>() -= removedLocalTotal.at(it->operRight.depth);
						removedLocalTotal.at(it->operRight.depth) = 0;
					}
				}
				if (it->operLeft.depth > 0 && it->operLeft.IsVariable())
				{
					AdjustOperandIndex(it->operLeft, removedLocal,block);
				}
				if (it->result.depth > 0 && it->result.IsVariable() && it->instrType == TokenType::EQUAL)
				{
					AdjustOperandIndex(it->result, removedLocal,block);
				}
				if (it->operRight.IsVariable() && isBranchType(it->instrType))
				{
					AdjustOperandIndex(it->operRight, removedLocal,block);
				}
				else if (it->result.IsVariable() && it->result.depth > 0 && 
					(it->instrType == TokenType::MINUS_MINUS
						|| it->instrType == TokenType::PLUS_PLUS))
				{
					AdjustOperandIndex(it->result, removedLocal,block);
				}
				else if (it->result.depth > 0 && (it->instrType == TokenType::LEFT_PAREN || it->instrType == TokenType::VAR))
				{
					if (it->result.IsVariable())
					{
						AdjustOperandIndex(it->result, removedLocal,block);
					}

				}
				else if (it->operRight.IsVariable())
				{
					AdjustOperandIndex(it->operRight, removedLocal,block);
				}
				it++;
			}
		}
		
		for (auto b : block->blocks)
		{
			if (!b->isSweeped)
			Sweep(b);
		}
		if (block->merge)
		{
			if (!block->merge->isSweeped)
			Sweep(block->merge);
		}
}




// Set to keep track of variables already in the worklist
std::set<std::pair<int, std::string>> workListSet;

void CalculateConstant(TokenType op, Operand& left, Operand& right, Instruction& instr,LatticeMap& value)
{
	const std::string& varName =  instr.result.IsTemp() ? 
		instr.result.GetTempName() :
		instr.result.GetVariableVerName();
	int depth = instr.result.depth;
	ValueContainer resultValue;

	switch (op)
	{
	case TokenType::PLUS:
		resultValue = ValueContainer::Add(left.value, right.value);
		break;
	case TokenType::MINUS:
		resultValue = ValueContainer::Substract(left.value, right.value);
		break;
	case TokenType::STAR:
		resultValue = ValueContainer::Multiply(left.value, right.value);
		break;
	case TokenType::PERCENT:
		resultValue = ValueContainer::Percent(left.value, right.value);
		break;
	case TokenType::SLASH:
		//// Handle division by zero
		//if (right.value.IsZero()) {
		//	// Set the variable's lattice value to BOTTOM
		//	value[{depth, varName}].type = LatticeValueType::BOTTOM;
		//	return;
		//}
		resultValue = ValueContainer::Divide(left.value, right.value);
		break;
	case TokenType::GREATER:
		resultValue = ValueContainer::Greater(left.value, right.value);
		break;
	case TokenType::GREATER_EQUAL:
	{
		resultValue = ValueContainer::Less(left.value, right.value);
		resultValue = !resultValue.AsRef<bool>();
	}
		break;
	case TokenType::LESS:
		resultValue = ValueContainer::Less(left.value, right.value);
		break;
	case TokenType::LESS_EQUAL:
	{
		resultValue = ValueContainer::Greater(left.value, right.value);
		resultValue = !resultValue.AsRef<bool>();
	}
	break;
	case TokenType::EQUAL_EQUAL:
		resultValue = ValueContainer::Equal(left.value, right.value);
		break;
	case TokenType::BANG_EQUAL:
	{
		resultValue = ValueContainer::Equal(left.value, right.value);
		resultValue = !resultValue.AsRef<bool>();
		break;
	}

	default:
		assert(false && "unknown binary operation on literals");
		return;
	}

	// Update the instruction and lattice value
	instr.operRight.value = resultValue;
	instr.result.isConstant = true;
	value[{depth, varName}].value = resultValue;
	value[{depth, varName}].type = LatticeValueType::CONSTANT;
}

void CFG::ConstPropagation()
{
	std::deque<std::pair<int, std::string>> workList;
	// Initialize the worklist with variables assigned constants
	for (auto& [varKey, defsBlocks] : localAssigned)
	{
		for (auto block : defsBlocks)
		{
			for (auto& [defVarKey, indices] : block->defs)
			{
				for (auto idx : indices)
				{

					auto& instr = block->instructions[idx];
					auto ssaKey = std::pair{ instr.result.depth,instr.result.GetVariableVerName() };
					bool isSimpleAssign = (instr.instrType == TokenType::EQUAL || instr.instrType == TokenType::DECLARE);
					if (isSimpleAssign && instr.operRight.isConstant)
					{
						auto& latticeVal = value[ssaKey];
						if (latticeVal.type != LatticeValueType::BOTTOM)
						{
							//instr.operRight.value.UpdateType(instr.result.type);

							latticeVal.value = instr.operRight.value;
							//instr.result.type = latticeVal.value.type;
							//instr.returnType = latticeVal.value.type;

							latticeVal.type = LatticeValueType::CONSTANT;
							if (workListSet.find(ssaKey) == workListSet.end())
							{
								workList.push_back(ssaKey);
								workListSet.insert(ssaKey);
							}
						}
						//else if (instr.operRight.value != LatticeValueType::CONSTANT)
						//{
						//	if (latticeVal.value != instr.operRight.value)
						//	{
						//		latticeVal.type = LatticeValueType::BOTTOM;
						//	}
						//}
					}
					// the value is not known
					else
					{
						bool isComplexAssign = (int)instr.instrType >=
											(int)TokenType::PLUS_EQUAL and (int)instr.instrType <=
											(int)TokenType::STAR_EQUAL  
											/* ||(int)instr.instrType >=
											(int)TokenType::PLUS_PLUS and (int)instr.instrType <=
											(int)TokenType::MINUS_MINUS*/;


						if ( instr.result.IsVariable() && workListSet.find(ssaKey) == workListSet.end())
						{
							if (value.find(ssaKey) == value.end()){
								value[ssaKey].type = LatticeValueType::TOP;
							}
							if (value[ssaKey].type == LatticeValueType::CONSTANT)
							{
								workList.push_back(ssaKey);
								workListSet.insert(ssaKey);

							}
						}
					}
				}
			}
		}
	}
	workListSet.clear();
	for (auto& [key, func] : functionCFG)
	{
		ConstProp(func.start, workList);
	}
}

std::string removeVersion(const std::string& input)
{
	size_t pos = input.find('_');
	return (pos != std::string::npos) ? input.substr(0, pos) : input;
}
std::string getVersion(const std::string& input)
{
	size_t pos = input.find('_');
	return (pos != std::string::npos) ? input.substr(pos, input.size()) : input;
}
std::pair<int, std::string> GetLatest(LatticeMap& value, int depth, const std::string& name)
{
	assert(depth >= 0);
	std::pair<int, std::string> latest;
	auto origName = removeVersion(name);
	auto origVersion= getVersion(name);
	auto it = value.find(std::pair{ depth, name });
	if (it != value.end()) return it->first;
	for (auto& [k, v] : value)
	{
		auto vers = getVersion(k.second);
		if (k.first <= depth && 
			removeVersion(k.second) == origName
			&& v.type == LatticeValueType::CONSTANT)
		{
			latest = k;
			origVersion = vers;
		}
	}
	return latest;
}

bool CFG::UpdateOperand(Operand& op)
{
	if (op.value.type == ValueType::NIL)
		return false;

	auto name = op.IsTemp() ? op.GetTempName() : op.GetVariableVerName();
	auto varKey = std::make_pair(op.depth, name);
	varKey = GetLatest(value, op.depth, name);
	auto it = value.find(varKey);
	if (it != value.end() && it->second.type == LatticeValueType::CONSTANT)
	{
		if (op.isConstant)
		{
			auto prevValue = op.value;
			if (prevValue == it->second.value) return false;
			op.value = it->second.value;
			return true;
		}
		else
		{
			op.value = it->second.value;
			op.type = op.value.type;
			op.isConstant = true;
			return true;
		}
		return false;
	}
	return false;
}


void CFG::PropagateTempValues(Block* block, size_t startIndex, Instruction& initialInstr, std::deque<std::pair<int, std::string>>& workList)
{
	auto iterInstr = initialInstr;
	size_t iterIndex = startIndex + 1;

	while (iterIndex < block->instructions.size())
	{
		auto& nextInstr = block->instructions[iterIndex];

		if (nextInstr.instrType == TokenType::BRANCH_ELIF || nextInstr.instrType == TokenType::BRANCH || nextInstr.instrType == TokenType::DECLARE || nextInstr.instrType == TokenType::EQUAL)
			break;

		auto tempValue = value.at({ iterInstr.result.depth, iterInstr.result.GetTempName() }).value;
		auto tempOper = iterInstr.result;
		tempOper.value = tempValue;
		// the iter instr oper right contains new updated value 
		// that will be used to calcuate new values
		if (nextInstr.operRight.originalName == iterInstr.result.originalName)
		{
			CalculateConstant(nextInstr.instrType, nextInstr.operLeft, iterInstr.operRight, nextInstr,value);
		}
		else if (nextInstr.operLeft.originalName == iterInstr.result.originalName)
		{
			CalculateConstant(nextInstr.instrType, iterInstr.operRight, nextInstr.operRight, nextInstr,value);
		}

		iterInstr = nextInstr;
		++iterIndex;
	}

	if (block->instructions[iterIndex].instrType 
		== TokenType::BRANCH
		|| block->instructions[iterIndex].instrType
		== TokenType::BRANCH_ELIF)
	{
		auto& newCheck = block->instructions[iterIndex - 1].operRight;
		block->instructions[iterIndex].operRight = block->instructions[iterIndex - 1].operRight;
		// once we propagated constant 
		// we don't need the calculation of condition
		// it is just been calculated
		block->markAll = false;
		return;
	}

	// If the result is a variable, update the value mapping
	if (iterIndex < block->instructions.size())
	{
		auto& nextInstr = block->instructions[iterIndex];
		if (nextInstr.result.IsVariable())
		{
			auto resultVarKey = std::make_pair(nextInstr.result.depth, nextInstr.result.GetVariableVerName());
			nextInstr.operRight = block->instructions[iterIndex - 1].operRight;
			value[resultVarKey].value = block->instructions[iterIndex - 1].operRight.value;
			value[resultVarKey].type = LatticeValueType::CONSTANT;
			workList.push_back(resultVarKey);
		}
	}
}


void CFG::ConstProp(Block* b, std::deque<std::pair<int, std::string>>& workList)
{
	// Check if the block has already been visited
	if (visitedBlocks.find(b) != visitedBlocks.end())
		return;
	visitedBlocks.insert(b);

	// Process the worklist
	while (!workList.empty())
	{
		auto varKey = workList.front();
		workList.pop_front();

		const std::string& varName = varKey.second;
		auto origName = removeVersion(varName);
		auto usesIt = localUses.find(varName);
		if (usesIt == localUses.end())
			continue;

		auto& blocksWithVar = usesIt->second;
		for (auto block : blocksWithVar)
		{
			auto opIt = block->uses.find(varName);
			if (opIt == block->uses.end())
				continue;

			auto& instrIndices = opIt->second;
			for (auto idx : instrIndices)
			{
				if (value.at(varKey).type == LatticeValueType::BOTTOM)
					continue;

				auto& instr = block->instructions[idx];
				bool leftUpdated = false, rightUpdated = false;

				if (instr.instrType == TokenType::PLUS_PLUS || instr.instrType == TokenType::MINUS_MINUS)
				{
					// Handle increment and decrement operations
					instr.operRight.value = (instr.instrType == TokenType::MINUS_MINUS)
						? ValueContainer::Substract(value[varKey].value, ValueContainer{ 1 })
						: ValueContainer::Add(value[varKey].value, ValueContainer{ 1 });

					instr.instrType = TokenType::EQUAL;
					instr.operRight.type = instr.result.type;
					value[varKey].value = instr.operRight.value;

					workList.push_back({ instr.result.depth, instr.result.GetVariableVerName() });
					continue;
				}
				else
				{
					if (instr.operLeft.GetVariableVerName() == varName)
					leftUpdated = UpdateOperand(instr.operLeft);
					if (instr.operRight.GetVariableVerName() == varName)
					rightUpdated = UpdateOperand(instr.operRight);
				}


				// If both operands are constant, fold the instruction
				if (instr.operLeft.isConstant && instr.operRight.isConstant)
				{
					CalculateConstant(instr.instrType, instr.operLeft, instr.operRight, instr,value);

					if (instr.result.IsVariable())
					{
						workList.push_back({ instr.result.depth, instr.result.GetVariableVerName() });
					}
					else if (instr.result.IsTemp())
					{
						// Update temporary variables
						PropagateTempValues(block, idx, instr,workList);
					}
				}
				else
				{
					// Update the value mapping if the result is a variable and the right operand is constant
					if (instr.result.IsVariable() && instr.operRight.isConstant)
					{
						auto resultVarKey = std::make_pair(instr.result.depth, instr.result.GetVariableVerName());
						auto& prevValue = value[resultVarKey].value;

						value[resultVarKey].type = LatticeValueType::CONSTANT;
						value[resultVarKey].value = instr.operRight.value;

						if (prevValue != instr.operRight.value)
							workList.push_back(resultVarKey);
					}
				}
			}
		}
	}

	// Recursively process child blocks
	for (auto child : b->blocks)
	{
		ConstProp(child, workList);
	}
	if (b->merge != nullptr)
	{
		ConstProp(b->merge, workList);
	}
}



void CFG::DeadCode()
{

	for (auto& [b, v] : globalDefs)
	{
		for (auto& i : v)
		{
			i += b.first->offsetPhi;
		}
	}

	std::queue<std::pair<Block*,Instruction*>> workList;
	//mark
	for (auto [key, func] : functionCFG)
	{
		Mark(func.start, workList);
	}
	
	
	for (auto [key, func] : functionCFG)
	{
		Sweep(func.start);
	}
	return;
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

	auto exitBlock = tpgSort.back();
	// Initialize postdom sets
	for (Block* b : tpgSort) {
		if (b == exitBlock) {
			b->postdom = { b };
		}
		else {
			b->postdom = std::set<Block*>{ tpgSort.begin() ,tpgSort.end() }; // All blocks in the CFG
			if (b->merge)
			{
				b->postdom.insert(b->merge);
			}
		}
	}

	bool changed = true;
	while (changed) {
		changed = false;
		for (Block* b : tpgSort) {
			if (b == exitBlock) continue;
			std::set<Block*> new_postdom;
			new_postdom.insert(b);

			// Intersection of postdom(s) for all successors s of b
			std::set<Block*> intersection;
			bool first = true;
			auto blocks = b->blocks;
			if (b->merge)
			{
				blocks.push_back(b->merge);
			}
			for (Block* s : blocks) {
				if (first) {
					intersection = s->postdom;
					first = false;
				}
				else {
					std::set<Block*> temp;
					std::set_intersection(
						intersection.begin(), intersection.end(),
						s->postdom.begin(), s->postdom.end(),
						std::inserter(temp, temp.begin())
					);
					intersection = temp;
				}
			}
			new_postdom.insert(intersection.begin(), intersection.end());

			if (new_postdom != b->postdom) {
				b->postdom = new_postdom;
				changed = true;
			}
		}
	}

	for (auto it = tpgSort.rbegin(); it != tpgSort.rend(); ++it) {

		auto b = *it;
		// Get the strict post-dominators of b
		// all post-dominators except oneself
		std::set<Block*> strict_postdom = b->postdom;
		strict_postdom.erase(b);

		for (Block* d : strict_postdom) {
			bool isImmediate = true;

			// Check if d is post-dominated by any other post-dominator in strict_postdom
			for (Block* other_d : strict_postdom) {
				if (other_d == d) continue;
				if (other_d->postdom.find(d) != other_d->postdom.end()) {
					// If d is post-dominated by another post-dominator, it's not the immediate post-dominator
					isImmediate = false;
					break;
				}
			}

			if (isImmediate) {
				b->ipdom = d;
				break;
			}
		}
	}
	// Ensure post-dominators are already calculated for each block
	for (auto it = tpgSort.rbegin(); it != tpgSort.rend(); ++it) { // iterate in reverse
		Block* b = *it;
		auto blocks = b->blocks;
		if (b->merge)
		{
			blocks.push_back(b->merge);
		}
		if (blocks.size() >= 2) { // If b has multiple successors
			for (Block* s : blocks) {
				Block* runner = s;
				while (runner != b->ipdom) { // ipdom is the immediate post-dominator
					runner->rdf.insert(b);
					runner = runner->ipdom;
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
		auto merge = blockToPrint->merge;
		if (merge != nullptr && visitedBlocks.find(merge) == visitedBlocks.end())
		{
			blockQueue.push(merge);
			visitedBlocks.insert(merge);
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

	Bfs(functionCFG.at("global").start, PrintBlock);
}

void AddLocalInfo(std::unordered_map<std::string, std::vector<Block*>>& info, const std::string& name, Block* b)
{
	auto it = info.find(name);
	if (it != info.end())
	{
		if (std::find(it->second.begin(), it->second.end(), b) == it->second.end())
		{
			info[name].push_back(b);
		}
	}
	else
		info[name].push_back(b);
}
void  CFG::InitLocal(Operand& op,const std::string& name)
{
	variableCounterLocal[name] = 0;
	auto [isExist, index, depth] = currentScope->IsLocalExist(name, currentScope->depth);
	assert(isExist);
	if (writeToVariable)
	{
		AddLocalInfo(localAssigned,name,currentBlock);
		currentBlock->uses[{name}];
		localUses[name];
	}
	else
	{
		//AddUse(0, name, currentBlock->instructions.size());
		//localUses[name].push_back(currentBlock);
	}
	op.originalName = name;
	op.index = index;
	op.depth = depth;
	op.type = currentScope->GetType(name);
}
void CFG::InitGlobal(Operand& op,const std::string& name)
{
	if (writeToVariable)
	{
		AddLocalInfo(localAssigned, name, currentBlock);
		currentBlock->uses[{name}];
		localUses[name];
	}
	else
	{
		//AddUse(0,name,currentBlock->instructions.size());
		//localUses[name].push_back(currentBlock);
	}
	op.originalName = name;
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
	auto resName = left->value.AsString();
	writeToVariable = true;
	auto resOp = InitVariable(resName, left->depth);
	writeToVariable = false;
	Operand rightOp{};
	if (expr->right != nullptr)
	{
		rightOp = ConvertExpressionAST(expr->right.get());
		if (left->depth > 0)
		{
			if (isNotPop)
			{
				notPoped.insert(resName);
			}
			if (!isNotPop && notPoped.find(resName) == notPoped.end())
			{
				currentScope->currentPopAmount = std::min(currentScope->currentPopAmount + 1, currentScope->popAmount);
			}
		}
	}

	auto instruction = Instruction{ type,{},rightOp,resOp };
	if (resOp.depth == 0)
	{
		MakeCritical(instruction);
	}
	instruction.returnType = rightOp.type;
	currentBlock->instructions.push_back(instruction);

	AddDef(resOp.depth, left->value.AsString(), currentBlock->instructions.size() - 1,currentBlock);
	
}

void CFG::AddUse(int depth, const std::string& name, int index, Block* b)
{
	//if (depth > 0)
	//{
	//	currentBlock->uses[{name}].push_back(index + currentBlock->offsetPhi);
	//}
	//else
	//{
	//	currentBlock->uses[{name}].push_back(index + currentBlock->offsetPhi);
	//	//globalUses[{currentBlock, name}].push_back(index);
	//	localUses[ name].push_back(currentBlock);
	//}
	auto newIndex = index + b->offsetPhi;
	if (std::find(b->uses[{name}].begin(), b->uses[{name}].end(), newIndex)
		== b->uses[{name}].end()) {
		b->uses[{name}].push_back(newIndex);
		AddLocalInfo(localUses, name, b);
	}
}

void CFG::AddDef(int depth, const std::string& name, int index,Block* b)
{
	if (depth > 0)
	{
		b->defs[{depth,name}].push_back(index + currentBlock->offsetPhi);
	}
	else
	{
		b->defs[{depth,name}].push_back(index + currentBlock->offsetPhi);
		globalDefs[{b, name}].push_back(index);
	}

}

void CFG::CreateVariableFrom(const Node* tree, const Operand& rightOp)
{
	auto expr = tree->As<Expression>();
	// left is variable
	auto left = expr->left->As<Expression>();
	auto name = left->value.AsString();
	writeToVariable = true;
	auto resOp = InitVariable(name,left->depth);
	writeToVariable = false;
	auto instruction = Instruction{ TokenType::EQUAL,{},rightOp,resOp };
	if (resOp.depth == 0)
	{
		MakeCritical(instruction); 
	}
	currentBlock->instructions.push_back(instruction);
	AddDef(resOp.depth,left->value.AsString(), currentBlock->instructions.size() - 1, currentBlock);

}

Operand CFG::CreateTemp()
{
	Operand temp{ ValueContainer{ "t"} ,false,tempVersion++};
	temp.isTemp = true;
	temp.originalName = std::format("t_{}", tempVersion);
	if(currentScope == nullptr)
	temp.depth = 0;
	else 
	temp.depth = currentScope->depth;

	AddDef(temp.depth,std::format("t{}", tempVersion - 1) , currentBlock->instructions.size(),currentBlock);
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
	auto resType = HighestType(left.type, right.type);
	if (IsBinaryBoolOp(type))
	{
		resType = ValueType::BOOL;
	}
	GiveType(instr, res, resType);
	currentBlock->instructions.push_back(instr);
	return res;
}

Block* CFG::CreateMergeBlock(std::vector<Block*> parents)
{
	auto mergeName = std::format("[merge_{}]", std::to_string(Block::counterMerge++));
	auto merge = CreateBlock(currentFunc,mergeName, parents);
	for (auto parent : parents)
	{
		parent->blocks.push_back(merge);
	}
	return merge;
}
Instruction CreateBranchInstruction(TokenType branchType, Operand conditionOp, Block* trueTarget, Block* falseTarget) {
	Instruction branchInstruction = { branchType, {}, conditionOp, NullOperand() };
	branchInstruction.targets.push_back(trueTarget);
	branchInstruction.targets.push_back(falseTarget);
	return branchInstruction;
}
Instruction CreateJumpInstruction(TokenType jumpType, std::initializer_list<Block*> targets, Block* currentBlock)
{
	Instruction jumpInstruction = { jumpType, {}, {}, NullOperand() };
	for (auto target : targets)
	{
		jumpInstruction.targets.push_back(target);
	}
	currentBlock->instructions.push_back(jumpInstruction);
	return jumpInstruction;
}
// Helper function to create a block and add a jump to it from the current block
Block* CFG::CreateConditionBlock(const std::string& name,  Block* currentBlock) {
	auto conditionBlock = CreateBlock(currentFunc,name, { currentBlock });
	conditionBlock->markAll = true;
	currentBlock->blocks.push_back(conditionBlock);
	return conditionBlock;
}
void CFG::EmitPop(Block* currentBlock,int popAmount)
{	
	auto pops = Operand{ popAmount,false,NOT_INIT_VERSION };
	pops.depth = currentScope->depth;
	Instruction instr{ TokenType::BLOCK,{},pops,{} };
	MakeCritical(instr);
	currentBlock->instructions.push_back(instr);
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
	{
		
		Instruction command = Instruction{ type,{},Operand{ValueContainer{forDepth.top()},true,NOT_INIT_VERSION},NullOperand()};
		command.isCritical = true;
		assert(currentScope->prevScope != nullptr);
		assert(currentScope != nullptr);
		auto diffDepth = currentScope->depth - forDepth.top();
		auto total = 0;
		Scope* temp = currentScope;
		while (diffDepth >= 0)
		{
			total += temp->currentPopAmount;
			temp = temp->prevScope;
			diffDepth--;
		}
		EmitPop(currentBlock, total);
		// for 
		currentBlock->instructions.push_back(command);
		break;
	}
	case TokenType::FOR:
	{
		auto parentBlock = currentBlock;
		auto forNode = tree->AsMut<For>();
		auto prevScope = currentScope;
		currentScope = &forNode->initScope;
		auto forInitName = std::format("[for_init_{}]", Block::counterForInit++);
		auto init = CreateConditionBlock(forInitName, currentBlock);


		currentBlock = init;
		forDepth.push(currentScope != nullptr ? currentScope->depth : 0);
		isNotPop = true;
		ConvertStatementAST(forNode->init.get());
		isNotPop = false;
		// so we do not pop iterator
		

		auto forConditionName = std::format("[for_condition_{}]", Block::counterForCondition++);
		auto condition = CreateConditionBlock(forConditionName, init);
		auto forActionName = std::format("[for_action_{}]", Block::counterForAction++);
		auto action = CreateBlock(currentFunc,forActionName, { condition });
		action->markAll = true;
		//currentBlock = action;
		//currentBlock = condition;

		auto forBodyName = std::format("[for_body_{}]", Block::counterForBody++);
		auto body = CreateBlock(currentFunc,forBodyName, { condition});
		auto forAction = CreateJumpInstruction(TokenType::JUMP, { init }, parentBlock);
		auto jumpFor = CreateJumpInstruction(TokenType::JUMP_FOR, {action,condition, body}, init);

		condition->blocks.push_back(body);
		condition->blocks.push_back(action);



		
		currentBlock = condition;
		auto conditionOp = ConvertExpressionAST(forNode->condition.get());
		Instruction branch = CreateBranchInstruction(TokenType::JUMP_BRANCH,conditionOp,action, condition);
		branch.targets.push_back(body);
		condition->instructions.push_back(branch);

		currentBlock = action;
		ConvertStatementAST(forNode->action.get());
		currentBlock = body;
		auto prevprevScope = currentScope;
		currentScope = forNode->body->AsMut<Scope>();
 		ConvertStatementAST(forNode->body.get());

		
		assert(currentBlock->instructions.back().instrType == TokenType::BLOCK);

		auto jumpBack = Instruction{ TokenType::JUMP_BACK,{},{},{} };

		currentBlock->instructions.insert(currentBlock->instructions.end(), jumpBack);


		auto parents = { parentBlock,currentBlock,/*init*/ };
		auto merge = CreateMergeBlock(parents);
		init->instructions.back().targets.push_back(merge);
		// should not we emit block instr to merge block instead?
		// it breaks on for in for loop case
		currentBlock = merge;
		//currentBlock = merge;
		//currentBlock->instructions.push_back(forAction);
		EmitPop(currentBlock, currentScope->popAmount);
		forDepth.pop();
		currentBlock = merge;
		currentScope = prevScope;
		break;
	}
	case TokenType::FUN:
	{
		auto func = tree->As<FunctionNode>();
		Instruction funcInstr{ TokenType::FUN,{},Operand{func->name,false,2},{} };
		MakeCritical(funcInstr);
		currentBlock->instructions.push_back(funcInstr);
		auto prevFunc = currentFunc;
		currentFunc = func->name;
		auto funcBlock = CreateBlock(currentFunc,func->name, {currentBlock});
		funcBlock->markAll = true;
		auto bodyName = "[" + func->name + "_body" + "]";
		auto bodyBlock = CreateBlock(currentFunc, bodyName,{ funcBlock });
		funcBlock->blocks.push_back(bodyBlock);
		currentBlock->blocks.push_back(funcBlock);
		currentBlock->blocks.push_back(bodyBlock);
		
		//startBlock->blocks.push_back(funcBlock);
		
		currentBlock = funcBlock;

		//auto prevScope = currentScope;
		currentScope = const_cast<Scope*>(&func->paramScope);
		//currentScope->prevScope = prevScope;
		currentBlock = funcBlock;
		for (auto& arg : func->arguments)
		{
			ConvertStatementAST(arg.get());
		}
		currentBlock = bodyBlock;
		ConvertStatementAST(func->body.get());
		//currentScope = prevScope;
		currentFunc = prevFunc;
		currentBlock = functionCFG.at(currentFunc).current;
		break;
	}
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

		Block* prevConditionBlock = nullptr;
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
					auto conditionBlock = CreateBlock(currentFunc,conditionName, { currentBlock });
					parentBlock->instructions.back().targets.push_back(conditionBlock);

					///
					// to always have it in final code
					branchElif.isMarked = true;
					// to have condition calculation in final code, 
					// if there is no constant to propagate
					conditionBlock->markAll = true;
					///
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

		
		parentBlock->instructions.back().targets.push_back(els);
		if (prevConditionBlock)
		{
			prevConditionBlock->instructions.back().targets.push_back(els);
		}

		mergeParents.push_back(currentBlock);
		//mergeParents.push_back(parentBlock);







		auto merge = CreateBlock(currentFunc,mergeName, {});
		for (auto parent : mergeParents)
		{
			auto& instr = parent->instructions.back();
			assert(instr.instrType == TokenType::JUMP_BRANCH);
			//if(parent != parentBlock)
			instr.targets.push_back(merge);
			parent->merge = merge;
			merge->parents.push_back(parent);
		}
		then->merge = merge;
		els->merge = merge;

		for (auto& elif : elifBlocks)
		{
			elif->merge = merge;
			//elif->blocks.push_back(merge);
		}
		currentBlock = parentBlock;
		currentBlock = merge;
		//EmitPop(currentBlock,currentScope->popAmount);

		break;
	}
	
	case TokenType::PRINT:
	{
		auto value = ConvertExpressionAST(expr->left.get());
		Instruction instr{ type,{},value, CreateTemp()};
		MakeCritical(instr);
		currentBlock->instructions.push_back(instr);
		break;
	}
	case TokenType::RETURN:
	{
		auto tmp = currentScope;
		// becase the last scope is argument scope, before that is body scope
		auto total = 0;
		while (tmp != nullptr && tmp->prevScope != nullptr)
		{
			//EmitPop(currentBlock, tmp->currentPopAmount);
			total += tmp->currentPopAmount;
			tmp = tmp->prevScope;
		}
		auto value = ConvertExpressionAST(expr->left.get());
		Instruction instr{ type,Operand{ValueContainer{total},false,IS_TEMP},value, CreateTemp() };
		MakeCritical(instr);
		currentBlock->instructions.push_back(instr);


		break;
	}
	case TokenType::WHILE:
	{
		auto parentBlock = currentBlock;
		auto whileConditionName = std::format("[while_condition_{}]", Block::counterWhileCondition++);
		auto condition = CreateConditionBlock(whileConditionName, currentBlock);
		auto condJump = CreateJumpInstruction(TokenType::JUMP_WHILE, { condition }, currentBlock);

		currentBlock = condition;
		auto cond = ConvertExpressionAST(expr->left.get());
		auto whileBodyName = std::format("[while_body_{}]", Block::counterWhileBody++);
		auto body = CreateBlock(currentFunc,whileBodyName, {currentBlock});
		currentBlock->blocks.push_back(body);


		auto branch = CreateBranchInstruction(TokenType::BRANCH_WHILE, cond, body, {});

		currentBlock = body;
		forDepth.push(currentScope != nullptr ? (currentScope->depth+1) : 1);
		ConvertStatementAST(expr->right.get());
		currentBlock->instructions.
			push_back(condJump);

		auto mergeName = std::format("[merge_{}]", std::to_string(Block::counterMerge++));
		auto parents = { parentBlock,currentBlock };
		auto merge = CreateMergeBlock(parents);
		branch.targets[1] = merge;
		condition->instructions.push_back(branch);


		Instruction jumpBack= Instruction{ TokenType::JUMP_BRANCH,{} ,{},NullOperand() };
		jumpBack.targets.push_back(parentBlock);
		merge->instructions.push_back(jumpBack);
		currentBlock = merge;
		forDepth.pop();
		break;
	}
	case TokenType::BLOCK:
	{
		auto block = currentBlock;
		auto scope = tree->AsMut<Scope>();
		currentScope = scope;
		for (auto& statement : scope->expressions)
		{
			ConvertStatementAST(statement.get());
		}

		auto acc = 0;
		block = currentBlock;
		while (block->instructions.size() > 0 && block->instructions.back().instrType == TokenType::BLOCK)
		{
			acc += block->instructions.back().operRight.value.As<int>();
			block->instructions.pop_back();
		}
		// to know how many times we need to pop
		EmitPop(block, scope->popAmount+acc);
		if (currentScope->prevScope)
		{
			currentScope = currentScope->prevScope;
		}
		
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
	case TokenType::LEFT_PAREN:
	{
		auto call = static_cast<const Call*>(tree);
		std::vector<Operand> args;
		args.reserve(call->args.size());
		auto res = CreateTemp();
		Operand  funcNameOp{ {call->name},false,2 };
		auto funcCall = Instruction{ TokenType::LEFT_PAREN,{},funcNameOp,res };
		static int counter = 0;
		auto name = std::format("args_{}", counter++);
		funcCall.argBlock = &graph[name];
		funcCall.argBlock->name = name;
		funcCall.argBlock->parents = {currentBlock};
		funcCall.argBlock->markAll = true;
		GiveType(funcCall, res, vm->GetGlobalType(call->name));
		currentBlock->instructions.push_back(funcCall);
		auto funcCallIndex = currentBlock->instructions.size() - 1;
		auto prevBlock = currentBlock;
		currentBlock = funcCall.argBlock;
		
		
		auto funcEntry = vm->GetGlobals().Get(call->name);
		auto funcValue = funcEntry->value.AsFunc();
		getAsParam = true;
		for (auto i = 0; i < call->args.size(); i++)
		{
			paramType = funcValue->argTypes[i];
			auto& arg = call->args[i];
			auto argOp = ConvertExpressionAST(arg.get());

			Operand actual = Operand{ ValueContainer{argOp.type},false,2 };
			Operand decl = Operand{ ValueContainer{funcValue->argTypes[i]},false,2 };
			Instruction instr{ TokenType::VAR,actual,decl,argOp };
			currentBlock->instructions.push_back(instr);

			args.push_back(argOp);
		}
		getAsParam = false;
		prevBlock->instructions[funcCallIndex].operLeft = Operand{ ValueContainer{(int)call->args.size()},true,2 };
		//auto callInstr = Instruction{ TokenType::CALL ,{},funcNameOp,{} };
		//callInstr.variables = args;
		//GiveType(callInstr, res, vm->GetGlobalType(call->name));
		//currentBlock->instructions.push_back(callInstr);
		currentBlock = prevBlock;
		return res;
		break;
	}
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
			writeToVariable = true;
			auto prevIndex = currentBlock->instructions.size() - 1;
			auto var = ConvertExpressionAST(expr->left.get());
			writeToVariable = false;
			Instruction action{ type,{},{},var };
			if (var.depth == 0)
			{
				MakeCritical(action);
			}
			GiveType(action, action.result, expr->value.type);
			currentBlock->instructions.push_back(action);

			AddDef(var.depth, var.value.AsString(), currentBlock->instructions.size() - 1,currentBlock);
			//AddUse(var.depth, var.value.AsString(), currentBlock->instructions.size() - 1);
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
		op.depth = currentScope != nullptr ? currentScope->depth : 0;
		op.type = ValueType::INT;
		return op;
		break;
	}
	case TokenType::FLOAT_LITERAL:
	{
		auto number = expr->value.As<float>();
		Operand op{ number,true,0 };
		op.depth = currentScope != nullptr ? currentScope->depth : 0;
		op.type = ValueType::FLOAT;
		return op;
		break;
	}
	case TokenType::STRING_LITERAL:
	{
		auto number = expr->value.As<std::string>();
		Operand op{ number,true,0 };
		op.type = ValueType::STRING;
		op.depth = currentScope != nullptr ? currentScope->depth : 0;
		return op;
		break;
	}
	case TokenType::TRUE:
	case TokenType::FALSE:
	{
		auto value = expr->value.As<bool>();
		Operand op{ value,true,0 };
		op.type = ValueType::BOOL;
		op.depth = currentScope != nullptr ? currentScope->depth : 0;
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
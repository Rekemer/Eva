#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "AST.h"
#include "SSA.h"
#define DEBUG_STACK 1
#define DEBUG_TOKENS 0
#define SSA 1
#define CONSTANT_FOLD 0
std::tuple<ValueContainer,VirtualMachine> Compile(const char* line)
{
	
	Lexer parser;
	VirtualMachine vm;
	if (!parser.Parse(line, vm)) return { ValueContainer{} ,vm};
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
		tree.vm = &vm;
		tree.Build(ptr);
		if (tree.IsPanic())
		{
			panic = true;
			continue;
		}
		tree.TypeCheck(vm);
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
		return{};
	}
#if SSA
	CFG cfg;
	cfg.vm = &vm;
	//cfg.startBlock
	for (auto& tree : trees)
	{
		auto node = tree.GetTree();
		cfg.ConvertAST(node);
	}
	cfg.TopSort();
	cfg.BuildDominatorTree();
	cfg.BuildDF();
	cfg.InsertPhi();
	cfg.Debug();
	//return {};
#endif 

#if SSA
	vm.GenerateBytecodeCFG(cfg);
#else
	for (auto& tree : trees)
	{
		vm.GenerateBytecodeAST(tree.GetTree());
	}
#endif
	vm.Execute();
#if DEBUG_STACK
	std::cout << "STACK [ ";
	auto& stack = vm.GetStack();
	for (auto& v : stack)
	{
		std::cout << v << ", ";
	}
	std::cout << " ]\n";
#endif // DEBUG

	if (vm.GetStack().size() > 1)
	{
		auto& stack = vm.GetStack();
		auto res = stack[stack.size()-2];
		return { res,vm };
	}
	return { {},vm };
}

ValueContainer CompileTest(const char* line)
{
	auto [res, vm] = Compile(line);
	return res;
}
#define DEBUG 0
#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "AST.h"
#include "SSA.h"
std::tuple<ValueContainer,VirtualMachine> Compile(const char* line)
{
	
	Lexer parser;
	VirtualMachine vm;
	if (!parser.Parse(line, vm)) return { ValueContainer{} ,vm};
	auto& tokens = parser.GetTokens();
	#if DEBUG
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
		//tree.Fold();
		trees.push_back(std::move( tree));

	}

#if 1
	CFG cfg;
	cfg.vm = &vm;
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
	return {};
#endif // DEBUG

	
	for (auto& tree : trees)
	{
		vm.GenerateBytecode(tree.GetTree());
	}
	#if DEBUG
	Print(tree.GetTree());
	#endif // DEBUG
	if (panic)
	{
		return{};
	}
	vm.Execute();
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
#define DEBUG 0
#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "AST.h"
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
		trees.push_back(std::move( tree));

	}
	// type inference 


	#if DEBUG
	Print(tree.GetTree());
	#endif // DEBUG

	if (panic)
	{
		return{};
	}
	
	vm.GenerateBytecode(trees);
	vm.Execute();
	if (vm.GetStack().size() > 0)
	{
		auto res = vm.GetStack().back();
		return { res,vm };
	}
	return { {},vm };
}

ValueContainer CompileTest(const char* line)
{
	auto [res, vm] = Compile(line);
	return res;
}
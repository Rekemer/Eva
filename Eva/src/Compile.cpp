#define DEBUG 0
#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "AST.h"
ValueContainer Compile(const char* line)
{
	Lexer parser;
	VirtualMachine vm;
	if (!parser.Parse(line,vm)) return ValueContainer{};



	auto& tokens = parser.GetTokens();
	#if DEBUG
	for (auto token : tokens)
	{
		std::cout << tokenToString(token.type) << " ";
	}

	std::cout << "\n";
	#endif // DEBUG
	std::vector<AST> trees;
	Token* ptr = &tokens[0];
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
	//auto res = vm.GetStack().top();
	return {};
}




VirtualMachine CompileRetVM(const char* line)
{
	Lexer parser;
	VirtualMachine vm;
	if (!parser.Parse(line, vm)) return vm;



	auto& tokens = parser.GetTokens();
#if DEBUG
	for (auto token : tokens)
	{
		std::cout << tokenToString(token.type) << " ";
	}

	std::cout << "\n";
#endif // DEBUG
	std::vector<AST> trees;
	Token* ptr = &tokens[0];
	bool panic = false;
	while (ptr->type != TokenType::END)
	{
		AST tree;
		tree.vm = &vm;
		tree.Build(ptr);
		tree.TypeCheck(vm);
		if (tree.IsPanic())
		{
			panic = true;
		}
		trees.push_back(std::move(tree));

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
	return vm;
}

ValueContainer CompileTest(const char* line)
{
	auto vm = CompileRetVM(line);
	auto res = vm.GetStack().top();
	return res;
}
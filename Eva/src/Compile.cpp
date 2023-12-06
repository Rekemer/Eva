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
	while (ptr->type!=TokenType::END)
	{
		AST tree;
		tree.vm = &vm;
		tree.Build(ptr);
		tree.TypeCheck(vm);
		trees.push_back(std::move( tree));

	}
	// type inference 



	#if DEBUG
	Print(tree.GetTree());
	#endif // DEBUG

	
	vm.GenerateBytecode(trees);

	vm.Execute();
	auto res = vm.GetStack().top();
	return res;
}
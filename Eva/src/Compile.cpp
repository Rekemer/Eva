#define DEBUG 0
#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "Expression.h"
ValueContainer Compile(const char* line)
{
	Lexer parser;
	if (!parser.Parse(line)) return ValueContainer{};



	auto& tokens = parser.GetTokens();
	#if DEBUG
	for (auto token : tokens)
	{
		std::cout << tokenToString(token.type) << " ";
	}

	std::cout << "\n";
	#endif // DEBUG

	Token* ptr = &tokens[0];
	AST tree;
	tree.Build(ptr);
	#if DEBUG
	Print(tree.GetTree());
	#endif // DEBUG

	VirtualMachine vm;
	vm.GenerateBytecode(tree);

	vm.Execute();
	auto res = vm.GetStack().top();
	return res;
}
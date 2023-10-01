#define DEBUG 0
#include "Compile.h"
#include  <iostream>
#include "Lexer.h"
#include "VirtualMachine.h"
#include "Expression.h"
ValueContainer Compile(const char* line)
{
	Lexer parser;
	parser.Parse(line);



	auto tokens = parser.GetTokens();
	#if DEBUG
	for (auto token : tokens)
	{
		std::cout << tokenToString(token.type) << " ";
	}

	std::cout << "\n";
	#endif // DEBUG

	Token* ptr = &tokens[0];
	Expression* tree = ParseExpression(ptr);
	#if DEBUG
	Print(tree);
	#endif // DEBUG

	VirtualMachine vm;
	vm.GenerateBytecode(tree);

	vm.Execute();
	auto res = vm.GetStack().top();
	return res;
}
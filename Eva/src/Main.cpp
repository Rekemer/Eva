#include "iostream"
#include "vector"
#include "Tokens.h"
#include "Expression.h"
#include "VirtualMachine.h"
#include "Lexer.h"
#include <stack>
#define DEBUG 1

int main(int argc, const char* argv[])
{
	
	std::string input;
	if (argc == 1)
	{
		// repl mode
		while (true)
		{
			std::cout << "> ";
			char line[1024];
			std::cin.getline(line, 1024);
			if (strcmp(line, "exit") == 0)
			{
				break;
			}
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
			Print(tree);

			VirtualMachine vm;
			vm.GenerateBytecode(tree);
			
			vm.Execute(); 
			auto res = vm.GetStack().top();
			std::cout << "result: " << res << std::endl;
		}
	}
	
	else
	{
		// parse file
	}


	std::cout << "Hello, Eva!\n";
}
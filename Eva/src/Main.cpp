#include "iostream"
#include "vector"
#include "Tokens.h"
#include "Expression.h"
#include "VirtualMachine.h"
#include "Parser.h"
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
			Parser parser;
			parser.Parse(line);

		

			const auto tokens = parser.GetTokens();
			#if DEBUG
				for (auto token : tokens)
				{
					std::cout << tokenToString(token.type) << " ";
				}

				std::cout << "\n";
			#endif // DEBUG
			VirtualMachine vm;
			Expression* tree = ParseExpression(tokens.data());
			Print(tree);

			vm.GenerateBytecode(tree);
			
			vm.Execute();
			std::cout << "result: " << vm.GetStack().top() << std::endl;
		}
	}
	
	else
	{
		// parse file
	}


	std::cout << "Hello, Eva!\n";
}
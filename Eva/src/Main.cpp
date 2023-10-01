#include "iostream"
#include "vector"
#include "Tokens.h"
#include "Expression.h"
#include "VirtualMachine.h"
#include "Lexer.h"
#include "Compile.h"
#include <stack>


int main(int argc, const char* argv[])
{
	
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
			auto res = Compile(line);
			std::cout << "result: " << res << std::endl;
		}
	}
	
	else
	{
		// parse file
	}


	std::cout << "Hello, Eva!\n";
}
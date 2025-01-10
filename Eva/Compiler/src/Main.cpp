#include "iostream"
#include "Compiler.h"
#include <fstream>
#include <sstream>

#define SCRIPT_PATH "test/"
int main(int argc, const char* argv[])
{
	Compiler compiler;
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
			auto res =  compiler.Compile(line);
			std::cout << "result: " << res << std::endl;
		}
	}
	else
	{
		
		// parse file
		auto scirptPath = std::string{ SCRIPT_PATH } + std::string{ argv[1] };
		std::ifstream scriptFile(scirptPath.data());

		if (scriptFile.is_open())
		{
			std::stringstream sstream;
			sstream << scriptFile.rdbuf();
			std::string contents = sstream.str();
			
			auto ret = compiler.Compile(contents.data());
			return ret;
		}
		scriptFile.close();
	}

}
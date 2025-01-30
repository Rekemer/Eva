#include "iostream"
#include "Compiler.h"
#include "ParseArg.h"
#include <fstream>
#include <sstream>
int main(int argc, const char* argv[])
{
	using namespace Eva;
	auto args = ParseArgsCmp(argc,argv);
	auto script = std::find_if(args.begin(), args.end(),
		[](const Arg& arg) { return arg.type == ArgType::EVA_PATH; });
	auto bytecodePath= std::find_if(args.begin(), args.end(),
		[](const Arg& arg) { return arg.type == ArgType::EVC_PATH; });
	
	assert(script != args.end());
	assert(bytecodePath != args.end());

	auto& scirptPath = script->value ;
	auto& binaryPath = bytecodePath->value;

	Compiler compiler{ scirptPath ,binaryPath };

	std::ifstream scriptFile(scirptPath.data());

	if (scriptFile.is_open())
	{
		std::stringstream sstream;
		sstream << scriptFile.rdbuf();
		std::string contents = sstream.str();
		
		auto ret = compiler.Compile(contents.data());
		return ret;
	}
	else
	{
		std::cerr << "sciprt path is not opened\n";
	}
	scriptFile.close();
	

}
#include "Serialize.h"
#include <fstream>
#include <memory>
#include "Function.h"
#include "VirtualMachine.h"
#include "ParseArg.h"



template <typename T>
void Execute(T& iarchive, bool isTest)
{
	VirtualMachine vm;
	iarchive(*vm.globalFunc);
	iarchive(vm.GetGlobals());
	vm.isTest = isTest;
	vm.Execute();
	if (vm.isTest)
	{
		//vm.GetGlobals().Print();
		vm.DumpGlobalToFile(".\\dumpGlobal.json");
	}
	
}

int main(int argc, const char* argv[])
{
	auto args = ParseArgsVM(argc, argv);
	auto binaryPath = args[args.size() - 1].value;
	auto isJson = HasJsonExtension(binaryPath);
	bool isTest = args[0].type == ArgType::TEST;

	std::cerr << "binary path" << binaryPath << std::endl;
	std::cerr << "isJson " << isJson << std::endl;
	std::cerr << "isTest " << isTest << std::endl;
	if (isJson)
	{
		std::ifstream os(binaryPath);
		if (!os.is_open())
		{
			std::cerr << "evc file is not found: " << binaryPath << std::endl;
			return -1;
		}
		cereal::JSONInputArchive iarchive{ os };
		Execute(iarchive, isTest);
		os.close();
	}
	else
	{
		std::ifstream os(binaryPath, std::ios::binary);
		if (!os.is_open())
		{
			std::cerr << "json file is not found: " << binaryPath << std::endl;
			return -1;
		}
		cereal::BinaryInputArchive iarchive{ os };
		Execute(iarchive, isTest);
		os.close();
	}
	return 0;
}
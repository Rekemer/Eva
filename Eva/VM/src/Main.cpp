#include "Serialize.h"
#include <fstream>
#include <memory>
#include "Function.h"
#include "VirtualMachine.h"
#include "ParseArg.h"


template <typename T>
void Execute(T& iarchive, bool isTest, std::string_view filepath)
{
	using namespace Eva;
	VirtualMachine vm;
	iarchive(*vm.globalFunc, vm.GetGlobals(), vm.functionNames,vm.pluginTable);


	auto e = vm.GetGlobals().Get("main");
	if (e->IsInit())
	{
		vm.mainFunc = e->value.AsFunc().get();
	}
	vm.isTest = isTest;
	vm.Execute();
	if (vm.isTest)
	{
		//vm.GetGlobals().Print();
		vm.DumpGlobalToFile(".\\dumpGlobal.json");



		if (vm.vmStack.size() > 1)
		{
			std::ofstream os("stack_check.txt", std::ios::app); // Open file in append mode
			if (!os) {
				std::cerr << "Failed to open file for appending." << std::endl;
				return;
			}
			std::string msg = std::format("{} has {} elements on stack after execution\n", filepath, vm.vmStack.size());
			os << msg; 
		}
		
	}
	
}

int main(int argc, const char* argv[])
{
	using namespace Eva;
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
		Execute(iarchive, isTest, binaryPath);
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
		Execute(iarchive, isTest, binaryPath);
		os.close();
	}
	return 0;
}
#include "Serialize.h"

#include <fstream>
#include <memory>

#include "Function.h"
#include "VirtualMachine.h"
#include "ParseArg.h"




//std::string replaceExtension(const std::string& binPath, const std::string& /newExtension) /{
//	size_t pos = binPath.rfind('.');
//	if (pos == std::string::npos) {
//		// No extension found, append the new extension
//		return binPath + "." + newExtension;
//	}
//	return binPath.substr(0, pos) + "." + newExtension;
//}
int main(int argc, const char* argv[])
{
	auto args = ParseArgsVM(argc, argv);
	auto binaryPath = args[args.size()- 1].value;
#if BIN
	std::ifstream os(binaryPath, std::ios::binary);
	if (!os.is_open())
	{
		std::cerr << "evc file is not found: " << binaryPath << std::endl;
		return -1;
	}
	cereal::BinaryInputArchive iarchive{os};
#else
	std::ifstream os("./../../Intermediates/test.json");
	cereal::JSONInputArchive iarchive{os};
#endif
	std::unique_ptr<Func>func = std::make_unique<Func>();
	if (os.is_open())
	{

		iarchive(*func);
		VirtualMachine vm;
		iarchive(vm.GetGlobals());
		vm.isTest = args[0].type == ArgType::TEST;
		std::cerr << std::format("isTest: {}", vm.isTest);
		vm.globalFunc = std::move(func);
		vm.Execute();
		if (vm.isTest)
		{
			vm.DumpGlobalToFile(".\\dumpGlobal.json");
		}
		os.close();
	}
	else
	{
		std::cout << "test.evc is not opened\n";
		return -1;
	}
	return 0;
}
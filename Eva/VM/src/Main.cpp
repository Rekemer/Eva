#include "Serialize.h"

#include <fstream>
#include <memory>

#include "Function.h"
#include "VirtualMachine.h"


enum class ArgType
{
	TEST,
	EVC_PATH,
};

struct Arg
{
	ArgType type;
	std::string value;
};


std::vector<Arg> ParseArgs(int argc, const char* argv[])
{
	std::vector<Arg> args;
	args.reserve(argc);
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (i + 1 != argc)
		{
			assert(arg[0] == '-');
			arg = arg.substr(1, arg.size() - 1);
		}
		
		if (arg == "test")
		{
			args.emplace_back(ArgType::TEST);
		}
		else
		{
			args.emplace_back(ArgType::EVC_PATH, arg);
		}
	}
	return args;
}

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
	auto args = ParseArgs(argc, argv);
	//auto binPath= std::string{ argv[1] };
#if BIN
	std::ifstream os("./../../Intermediates/test.evc", std::ios::binary);
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
		vm.globalFunc = std::move(func);
		vm.Execute();
		vm.DumpGlobalToFile("./../../Intermediates/dumpGlobal.json");
		os.close();
	}
	else
	{
		std::cout << "test.evc is not opened\n";
		return -1;
	}
	return 0;
}
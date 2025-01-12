#include "Serialize.h"

#include <fstream>
#include <memory>

#include "Function.h"
#include "VirtualMachine.h"
std::string replaceExtension(const std::string& binPath, const std::string& newExtension) {
	size_t pos = binPath.rfind('.');
	if (pos == std::string::npos) {
		// No extension found, append the new extension
		return binPath + "." + newExtension;
	}
	return binPath.substr(0, pos) + "." + newExtension;
}
int main(int argc, const char* argv[])
{
	//auto binPath= std::string{ argv[1] };
#if BIN
	std::ifstream os("./../../Intermediates/test.evc");
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
		os.close();
	}
	else
	{
		std::cout << "test.evc is not opened\n";
		return -1;
	}
	return 0;
}
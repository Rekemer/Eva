#include"PluginLoader.h"
namespace Eva
{
	DLLHandle CastToModule(void* ptr)
	{
		return reinterpret_cast<DLLHandle>(ptr);
	}

	std::pair<bool, std::string_view> IsPlugin(std::string_view functionName, std::unordered_map<std::string, PluginData>& plugins)
	{
		bool isPlugin = false;
		std::string wrapperName = "wrapper_" + std::string{ functionName };
		std::string_view pluginName;
		for (auto& [name, data] : plugins)
		{
			auto hModule = static_cast<DLLHandle>(data.hDLL);
			auto funcAddress = GetProcAddress(hModule, wrapperName.data());
			if (funcAddress)
			{
				isPlugin = true;
				pluginName = name;
			}
		}
		return { isPlugin, pluginName };
	}
}
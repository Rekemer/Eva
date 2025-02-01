#include "Native.h"
#include "ICallable.h"
#include "Function.h"
#include "PluginLoader.h"
namespace Eva
{
	void Wrapper_Print(std::vector<ValueContainer> args)
	{
		for (auto& arg : args)
		{
			std::cout << arg << " ";
		}
		std::cout << std::endl;
	}

	std::unordered_map<std::string_view, NativeWrapper> native
	{
		{"Print",Wrapper_Print}
	};

	std::unordered_map<std::string_view, std::shared_ptr<NativeFunc>> nativeCalls
	{
		{"Print",std::make_shared<NativeFunc>(std::vector<ValueType>{}, ICallable::INF_ARGS, "Print")}
	};
	std::unordered_map<std::string_view, ValueType> nativeReturnTypes
	{
		{"Print",ValueType::NIL}
	};
	NativeWrapper CallNative(std::string_view name)
	{
		return native.at(name);
	}

	std::pair<bool, std::string_view> IsPlugin(std::string_view functionName, std::unordered_map<std::string, PluginData>& plugins)
	{
		bool isPlugin = false;
		std::string wrapperName = "wrapper_" + std::string{ functionName };
		std::string_view pluginName;
		for (auto& [name, data] : plugins)
		{
			auto hModule = static_cast<HMODULE>(data.hDLL);
			auto funcAddress = GetProcAddress(hModule, wrapperName.data());
			if (funcAddress)
			{
				isPlugin = true;
				pluginName = name;
			}
		}
		return { isPlugin, pluginName };
	}

	bool IsNative(std::string_view str)
	{
		return native.find(str) != native.end();
	}

	std::shared_ptr<ICallable> GetNative(std::string_view name)
	{
		return nativeCalls.at(name);
	}
	ValueType GetNativeType(std::string_view name)
	{
		return nativeReturnTypes.at(name);
	}
}


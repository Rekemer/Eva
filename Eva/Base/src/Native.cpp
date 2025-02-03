#include "Native.h"
#include "ICallable.h"
#include "Function.h"
#include "PluginLoader.h"
#include "CallState.h"
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
	

	bool IsNative(std::string_view str)
	{
		return native.find(str) != native.end();
	}

	std::shared_ptr<ICallable> GetNativeCall(std::string_view name)
	{
		return nativeCalls.at(name);
	}

	NativeWrapper GetNative(std::string_view name)
	{
		return native.at(name);
	}

	ValueType GetNativeType(std::string_view name)
	{
		return nativeReturnTypes.at(name);
	}
}


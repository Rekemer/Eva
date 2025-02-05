#include "Native.h"
#include "ICallable.h"
#include "Function.h"
#include "PluginLoader.h"
#include "CallState.h"
namespace Eva
{
	int Wrapper_Print(CallState& callState)
	{
		std::vector<ValueContainer> args;

		for (auto i = 0; i < callState.argumentCount; i++)
		{
			args.push_back(callState.stack.back());
		}
		for (auto i = args.end();  i!= args.begin(); i--)
		{
			std::cout << *i<< " ";
		}
		for (auto i = 0; i < callState.argumentCount; i++)
		{
			callState.stack.pop_back();
		}
		std::cout << std::endl;
		return 0;
	}


	std::unordered_map<std::string_view, std::shared_ptr<NativeFunc>> nativeCalls
	{
		{"Print",std::make_shared<NativeFunc>(std::vector<ValueType>{},
			Wrapper_Print,ICallable::INF_ARGS, "Print")}
	};
	std::unordered_map<std::string_view, ValueType> nativeReturnTypes
	{
		{"Print",ValueType::NIL}
	};
	
	bool IsNative(std::string_view str)
	{
		return nativeCalls.find(str) != nativeCalls.end();
	}
	std::shared_ptr<ICallable> GetNativeCall(std::string_view name)
	{
		return nativeCalls.at(name);
	}
	ValueType GetNativeType(std::string_view name)
	{
		return nativeReturnTypes.at(name);
	}
}


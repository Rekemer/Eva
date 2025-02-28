#include "Native.h"
#include "ICallable.h"
#include "Function.h"
#include "PluginLoader.h"
#include "CallState.h"
namespace Eva
{
	// potentally add parsing of args to emulate pritnf()
	int Wrapper_Print(CallState& callState)
	{
		std::vector<ValueContainer> args;

		for (auto i = 0; i < callState.argumentCount; i++)
		{
			args.push_back(callState.stack.back());
			callState.stack.pop_back();
		}
		for (auto i = args.rbegin(); i != args.rend(); i++)
		{
			std::cout << *i << " ";
		}
		std::cout << std::endl;
		callState.stack.pop_back();
		return 0;
	}


	std::unordered_map<std::string_view, std::shared_ptr<NativeFunc>> nativeCalls
	{
		{"Print",std::make_shared<NativeFunc>(std::vector<ValueType>{},
			Wrapper_Print,ICallable::INF_ARGS, "Print", CallFlags::BuiltIn | CallFlags::VoidCall)}
	};
	std::unordered_map<std::string_view, ValueType> nativeReturnTypes
	{
		{"Print",ValueType::NIL}
	};

	bool IsNative(std::string_view str)
	{
		return nativeCalls.find(str) != nativeCalls.end();
	}
	eCallable GetNativeCall(std::string_view name)
	{
		return nativeCalls.at(name);
	}
	ValueType GetNativeType(std::string_view name)
	{
		return nativeReturnTypes.at(name);
	}

	void CallUserCallback(eCallable call, CallState* st, const std::vector<ValueContainer>& args)
	{
		auto argAmount = call->argCount;
		assert(st != nullptr);
		st->stack.push_back(call);
		for (auto& arg : args)
		{
			st->stack.push_back(arg);
		}
		st->argumentCount = argAmount;
		call->Call(*st, st->GetStartIndex());
	}
}


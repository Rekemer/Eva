#pragma once
#include "Function.h"
#include "Native.h"
#include "CallState.h"
#include "PluginLoader.h"
//#include "../../VM/src/VirtualMachine.h"
namespace Eva
{
	void CallNative(CallState& callState, std::string_view name)
	{
		auto& stack = callState.stack;

		std::vector<ValueContainer> args;
		auto argumentCount = callState.argumentCount;
		assert(argumentCount != -1);
		args.reserve(argumentCount);

		for (auto i = stack.size() - 1; i > stack.size() - 1 - argumentCount; i--)
		{
			args.insert(args.begin(), stack[i]);
		}

		return GetNative(name)(args);
	}
	DLLHandle GetPluginHandle(PluginTable& plugins, std::string_view pluginName)
	{
		auto& pluginData = plugins.at(pluginName.data());
		return CastToModule(pluginData.hDLL);
	}

	void CallPlugin(CallState& callState,
		std::string_view funcName)
	{
		//
		std::string pluginName;
		auto handle = GetPluginHandle(callState.pluginTable, pluginName);

		// cache load funcs


		//
		auto func = LoadFunc<DllWrapper>(handle, funcName);
		func(callState);

	}


	size_t Func::Call(CallState& callState, size_t baseIndex)
	{
		auto& frame = callState.callFrames[callState.nextToCurrentCallFrame++];
		frame.function = this;
		frame.ip = 0;
		frame.stackIndex = baseIndex;
		return callState.nextToCurrentCallFrame - 1;
	}

	size_t NativeFunc::Call(CallState& callState, size_t baseIndex)
	{
		
		assert(callFlags != CallFlags::UserFunc);

		if (callFlags == CallFlags::BuiltIn)
		{
			CallNative(callState, name);
		}
		else
		{
			CallPlugin(callState, name);
		}
		return SIZE_MAX;
	}
}
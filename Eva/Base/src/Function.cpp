#pragma once
#include "Function.h"
#include "Native.h"
#include "CallState.h"
#include "PluginLoader.h"
//#include "../../VM/src/VirtualMachine.h"
namespace Eva
{
	void CallNative(CallState& callState, NativeWrapper& func, std::string_view name)
	{
		auto& stack = callState.stack;

		std::vector<ValueContainer> args;
		auto argumentCount = callState.argumentCount;
		assert(argumentCount != -1);
		//args.reserve(argumentCount);
		//
		//for (auto i = stack.size() - 1; i > stack.size() - 1 - argumentCount; i--)
		//{
		//	args.insert(args.begin(), stack[i]);
		//}
		func(callState);
	}
	DLLHandle GetPluginHandle(PluginTable& plugins, std::string_view pluginName)
	{
		auto& pluginData = plugins.at(pluginName.data());
		return CastToModule(pluginData.hDLL);
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
		assert(!HasFlag(callFlags, CallFlags::UserFunc));
		CallNative(callState, func, name);
		return SIZE_MAX;
	}
}
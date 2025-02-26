#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <string_view>
#include "Value.h"
#include "PluginData.h"
#include "CallState.h"
namespace Eva
{
	class ICallable;

	// should not copy?
	using NativeWrapper = int(*)(CallState&);
	//using DllWrapper = int(*)(CallState&);


	// checks built in
	bool IsNative(std::string_view str);

	void CallUserCallback(eCallable call, size_t startIndex, CallState* st, const std::vector<ValueContainer>& args);
	
	eCallable GetNativeCall(std::string_view name);
	ValueType GetNativeType(std::string_view name);
}
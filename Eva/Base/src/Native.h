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
	using NativeWrapper = std::function<void(std::vector<ValueContainer>)>;
	


	// checks built in
	bool IsNative(std::string_view str);

	

	std::shared_ptr<ICallable> GetNativeCall(std::string_view name);
	NativeWrapper GetNative(std::string_view name);
	ValueType GetNativeType(std::string_view name);
}
#pragma once
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <string_view>
#include "Value.h"
#include "PluginData.h"
namespace Eva
{
	class ICallable;

	// should not copy
	using NativeWrapper = std::function<void(std::vector<ValueContainer>)>;

	NativeWrapper CallNative(std::string_view name);


	// checks built in
	bool IsNative(std::string_view str);

	std::pair<bool, std::string_view> IsPlugin(std::string_view functionName, std::unordered_map<std::string, PluginData>& plugins);

	std::shared_ptr<ICallable> GetNative(std::string_view name);
	ValueType GetNativeType(std::string_view name);
}
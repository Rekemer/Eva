#pragma once
#include <vector>
#include <memory>
#include <functional>
#include <string_view>
#include "Value.h"
class ICallable;

// should not copy
using NativeWrapper = std::function<void(std::vector<ValueContainer>)>;

NativeWrapper CallNative(std::string_view name);


bool IsNative(std::string_view str);


std::shared_ptr<ICallable> GetNative(std::string_view name);
ValueType GetNativeType(std::string_view name);
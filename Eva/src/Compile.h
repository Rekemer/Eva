#pragma once
#include "Value.h"
#include "VirtualMachine.h"
#include <tuple>
std::tuple<ValueContainer, VirtualMachine> Compile(const char* line);
ValueContainer CompileTest(const char* line);

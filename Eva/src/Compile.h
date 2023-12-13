#pragma once
#include "Value.h"
#include "VirtualMachine.h"
ValueContainer Compile (const char* line);
VirtualMachine CompileRetVM(const char* line);
#pragma once
#include"Object.h"
#include "VirtualMachine.h"
#include <vector>

struct FunctionContainer final: public Object
{
	int argCount;
	std::vector<Bytecode> opCode;
};
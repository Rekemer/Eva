#pragma once
#include"Object.h"
#include <vector>
#include "Bytecode.h"
#include "Value.h"

struct Func final: public Object
{
	int argCount;
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
};
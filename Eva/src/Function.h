#pragma once
#include"Object.h"
#include <vector>
#include "Bytecode.h"

struct Func final: public Object
{
	int argCount;
	std::vector<Bytecode> opCode;
};
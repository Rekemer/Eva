#pragma once
#include"Object.h"
#include"String.hpp"
#include <vector>
#include "Bytecode.h"
#include "Value.h"

struct Func final: public Object
{
	int argCount;
	String name;
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
};
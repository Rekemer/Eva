#pragma once
#include <vector>
#include <bitset>
#include"Object.h"
#include"String.hpp"
#include "Bytecode.h"
#include "Value.h"

struct Func final: public Object
{
	int argCount;
	std::vector<ValueType> argTypes;
	String name;
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
};
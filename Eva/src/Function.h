#pragma once
#include <vector>
#include <bitset>
#include <memory>
#include"Object.h"
#include <string>
#include "Bytecode.h"
#include "Value.h"

struct Func final: public Object
{
	int argCount;
	std::vector<ValueType> argTypes;
	std::string name;
	std::vector<Bytecode> opCode;
	std::vector<ValueContainer> constants;
};
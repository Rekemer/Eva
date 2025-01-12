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
	//template<class Archive>
	//void serialize(Archive& archive)
	//{
	//	// when we serialize an edge, we'll defer serializing the associated node
	//	// to avoid extensive recursive serialization
	//	auto& constPool = constants;
	//	auto& bytecode = opCode;
	//}
};

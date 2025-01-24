#pragma once
#include <vector>
#include <string>
#include "Value.h"
class VirtualMachine;
class ICallable
{
public:
	const inline static int INF_ARGS = -2;

	ICallable() = default;

	ICallable(std::vector<ValueType> argTypes, int argCount, std::string_view name)
		:argTypes{ argTypes },
		argCount{ argCount },
		name{name.data()}
	{

	}

	bool IsArgUnlimited()
	{
		return argCount == INF_ARGS;
	}

	virtual size_t Call(VirtualMachine& vm, size_t argumentCount, size_t baseIndex) = 0;
	int argCount = 0;
	std::vector<ValueType> argTypes;
	std::string name;
	virtual ~ICallable()
	{

	};
};




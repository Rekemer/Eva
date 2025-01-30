#pragma once
#include <vector>
#include <bitset>
#include <memory>
#include "ICallable.h"
#include <string>
#include "Bytecode.h"
#include "Value.h"
namespace Eva
{
	class Func final : public ICallable
	{
	public:
		size_t Call(VirtualMachine& vm, size_t argumentCount, size_t baseIndex) override;

		std::vector<Bytecode> opCode;
		std::vector<ValueContainer> constants;
	};
	class NativeFunc final : public ICallable
	{
	public:
		NativeFunc(std::vector<ValueType> args, int argAmount, std::string_view name) :
			ICallable{ args,argAmount,name }
		{

		};
		size_t Call(VirtualMachine& vm, size_t argumentCount, size_t baseIndex) override;
	};
}
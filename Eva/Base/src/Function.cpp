#pragma once
#include "Function.h"
#include "Native.h"
#include "../../VM/src/VirtualMachine.h"

size_t Func::Call(VirtualMachine& vm, size_t argumentCount, size_t baseIndex)
{
	auto& frame = vm.callFrames[vm.nextToCurrentCallFrame++];
	frame.function = this;
	frame.ip = 0;
	frame.stackIndex = baseIndex;
	return vm.nextToCurrentCallFrame - 1;
}

size_t NativeFunc::Call(VirtualMachine& vm, size_t argumentCount, size_t baseIndex)
{
	auto& stack = vm.vmStack;

	std::vector<ValueContainer> args;
	args.reserve(argumentCount);

	for (auto i = stack.size() - 1; i > stack.size() - 1 - argumentCount; i--)
	{
		args.insert(args.begin(),stack[i]);
	}
	CallNative(name)(args);
	return SIZE_MAX;
}
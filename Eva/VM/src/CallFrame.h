#pragma once
const int callFrameAmount = 64;
struct CallFrame
{
	Func* function;
	size_t ip = 0;
	// base pointer to stack
	// which is start of counting
	// for the function 
	int stackIndex = -1;
};
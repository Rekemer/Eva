#pragma once
#include <string>
#include "Value.h"
struct Entry
{
	std::string key = "";
	ValueContainer value;
	bool IsInit()
	{
		return key != "";
	}
};
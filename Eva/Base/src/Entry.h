#pragma once
#include <string>
#include "Value.h"
namespace Eva
{

	struct Entry
	{
		std::string key = "";
		ValueContainer value;
		bool IsInit()
		{
			return key != "";
		}
	};
}
#pragma once
#include<memory>
#include<ostream>
#include"Object.h"
// could be a template?
enum class ValueType
{
	FLOAT,
	BOOL,
	OBJ,
	NIL
};

struct ValueContainer
{
	ValueType type{ ValueType::NIL };
	ValueContainer() = default;
	explicit ValueContainer(bool v)
	{
		type = ValueType::BOOL;
		as.boolean = v;
	}
	explicit ValueContainer(float v)
	{
		type = ValueType::FLOAT;
		as.numberFloat = v;
	}
	union
	{
		bool boolean ;
		float numberFloat;
		Object* object;
	}as;
	
};
inline std::ostream& operator<<(std::ostream& os, const ValueContainer& v)
{
	switch (v.type)
	{
		case  ValueType::BOOL:
		{
			bool val = v.as.boolean;
			os << val;
			break;
		}
		case  ValueType::FLOAT:
		{
			float num = v.as.numberFloat;
			os << num;
			break;
		}
		default:
			break;
	}
	return os;
	
}
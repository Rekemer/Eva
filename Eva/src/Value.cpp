#include"Value.h"
#include<cassert>
#include"Object.h"
#include"String.hpp"
#include"Function.h"

const char* ValueToStr(ValueType valueType) {
switch (valueType)
{
	case ValueType::FLOAT:
		return "FLOAT";
	case ValueType::INT:
		return "INT";
	case ValueType::BOOL:
		return "BOOL";
	case ValueType::STRING:
		return "STRING";
	case ValueType::NIL:
		return "NIL";
	case ValueType::FUNCTION:
		return "FUNCTION";
	default:
		return "Unknown value type";
	}
}

ValueContainer::ValueContainer(const ValueContainer& v)
{
	type = v.type;
	as = v.as;
	if (v.type == ValueType::STRING)
	{
		as = new String(*static_cast<String*>(std::get<String*>(v.as)));
	}
};


ValueContainer::ValueContainer(ValueType v)
{
	type = v;
	if (v == ValueType::FUNCTION)
	{
		as = new Func();
	}
}
ValueContainer& ValueContainer::operator= (const ValueContainer& v)
{
	if (this == &v) return *this;
	type = v.type;
	as = v.as;
	if (v.type == ValueType::STRING && std::get<String*>(v.as) != nullptr)
	{

		as = new String(*static_cast<String*>(std::get<String*>(v.as)));
	}
}


std::ostream& operator<<(std::ostream& os, const ValueContainer& v)
{
	switch (v.type)
	{
	case  ValueType::BOOL:
	{
		bool val = std::get<bool>(v.as);
		os << val;
		break;
	}
	case  ValueType::FLOAT:
	{
		float num = std::get<float>(v.as);
		os << num;
		break;
	}
	case  ValueType::INT:
	{
		int num = std::get<int>(v.as);
		os << num;
		break;
	}
	case  ValueType::STRING:
	{
		auto str = std::get<String*>(v.as);
		os << *str;
		break;
	}
	case  ValueType::FUNCTION:
	{
		auto name = std::get<Func*>(v.as)->name;
		os << name;
		break;
	}
	default:
		break;
	}
	return os;

}

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
	if (v.type == ValueType::STRING)
	{
		as = std::make_shared<String>(*v.AsString());
	}
	else as = v.as;
};

void ValueContainer::UpdateType(ValueType type)
{
	if (type == ValueType::STRING)
	{
		as = std::make_shared<String>();
	}
	this->type = type;
	
}

ValueContainer::ValueContainer(ValueType v)
{
	type = v;
	if (v == ValueType::FUNCTION)
	{
		as = std::make_shared<Func>();
	}
	else if (v == ValueType::STRING)
	{
		as = std::make_shared<String>();
	}
}
ValueContainer& ValueContainer::operator= (const ValueContainer& v)
{
	if (this == &v) return *this;
	type = v.type;
	if (v.type == ValueType::STRING )
	{

		as = std::make_shared<String>(*v.AsString());
	}
	else as = v.as;
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
		auto str = v.AsString();
		os << *str;
		break;
	}
	case  ValueType::FUNCTION:
	{
		auto name =v.AsFunc()->name;
		os << *name;
		break;
	}
	default:
		break;
	}
	return os;

}

#include "Value.h"
#include <cassert>
#include "Object.h"
#include <string>
#include "Function.h"
#include "VirtualMachine.h"

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
	//if (v.type == ValueType::STRING)
	//{
	//	as = std::make_shared<String>(*v.AsString());
	//}
	//else as = v.as;
};



void ValueContainer::UpdateType(ValueType newType)
{
	if (newType == this->type) return;
	auto prevType = this->type;
	this->type = newType;
}
bool AreBothNumeric(ValueType left, ValueType right)
{
	switch (left)
	{
	case ValueType::FLOAT:
	case ValueType::INT:
		return (right == ValueType::FLOAT || right == ValueType::INT);
	default:
		return false;
	}
}
// turns out ternary operator can perform type promotion which causes int to be casted to float
// later on it causes exception at runtime
#define OP(v1, v2, op)                                         \
do {                                                           \
    bool isNumber = AreBothNumeric(v1.type, v2.type);           \
    assert(v1.type == v2.type || isNumber);                    \
    switch (v1.type)                                           \
    {                                                          \
    case ValueType::FLOAT:                                     \
        if (v2.type == ValueType::FLOAT) {                     \
            return ValueContainer{ v1.As<float>() op v2.As<float>() };  \
        } else {                                               \
            return ValueContainer{ v1.As<float>() op v2.As<int>() };    \
        }                                                      \
        break;                                                 \
    case ValueType::INT:                                       \
        if (v2.type == ValueType::FLOAT) {                     \
            return ValueContainer{ v1.As<int>() op v2.As<float>() };    \
        } else {                                               \
            return ValueContainer{ v1.As<int>() op v2.As<int>() };      \
        }                                                      \
        break;                                                 \
    case ValueType::BOOL:                                      \
        return ValueContainer{ v1.As<bool>() op v2.As<bool>() };\
        break;                                                 \
    case ValueType::FUNCTION:                                  \
    case ValueType::DEDUCE:                                    \
    case ValueType::NIL:                                       \
        assert(false && "Invalid operation on non-numeric type"); \
        break;                                                 \
    default:                                                   \
        break;                                                 \
    }                                                          \
    return {};                                                 \
} while (0)

ValueContainer ValueContainer::Add(const ValueContainer& v1, const ValueContainer& v2)
{
	bool isNumber = AreBothNumeric(v1.type, v2.type);
	assert(v1.type == v2.type || isNumber);
	switch (v1.type)
	{
	case ValueType::FLOAT:
		if (v2.type == ValueType::FLOAT) {
			
				return ValueContainer{ v1.As<float>() + v2.As<float>() };  
		}
		return ValueContainer{ v1.As<float>() + v2.As<int>() };    
		break;

	case ValueType::INT:
	{
		
		if (v2.type == ValueType::FLOAT) {
			return ValueContainer{ v1.As<int>() + v2.As<float>() };
		}
		return ValueContainer{ v1.As<int>() + v2.As<int>() };
		break;
	}

	case ValueType::STRING:
		return ValueContainer{ v1.AsString() + v2.AsString()};
		break;
	case ValueType::BOOL:
	case ValueType::FUNCTION:
	case ValueType::DEDUCE:
	case ValueType::NIL:
		assert(false);
	default:
		break;
	}
	return {};
}

void ValueContainer::InverseBool()
{
	auto isBool = type == ValueType::BOOL;
	assert(isBool);
	auto& v = std::get<bool>(as);
	v = !v;
}
void ValueContainer::Negate()
{
	auto isInt = type == ValueType::INT;
	auto isFloat = type == ValueType::FLOAT;
	assert(isInt|| isFloat);
	if (isInt)
	{
		auto v = std::get<int>(as);
		as = -v;
	}
	else
	{
		auto v = std::get<float>(as);
		as = -v;
	}
}

ValueContainer ValueContainer::Substract(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, -);
}

ValueContainer ValueContainer::Divide(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, /);
}
ValueContainer ValueContainer::Percent(const ValueContainer& v1, const ValueContainer& v2)
{
	assert(v1.type == ValueType::INT);
	assert(v2.type == ValueType::INT);
	return v1.As<int>() % v2.As<int>();
	//OP(v1, v2, %);
}
ValueContainer ValueContainer::Multiply(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, *);
}
ValueContainer ValueContainer::And(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, &&);
}

ValueContainer ValueContainer::Or(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, ||);
}
ValueContainer ValueContainer::Greater(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, > );
}
ValueContainer ValueContainer::Less(const ValueContainer& v1, const ValueContainer& v2)
{
	OP(v1, v2, < );
}
ValueContainer ValueContainer::Equal(const ValueContainer& v1, const ValueContainer& v2)
{
	if (v1.type == v2.type  && v1.type == ValueType::STRING)
	{
		return ValueContainer { v1.AsString() == v2.AsString() };
	}
	OP(v1, v2, == );
}

ValueContainer::ValueContainer(ValueType v)
{
	type = v;
	if (v == ValueType::FUNCTION)
	{
		as = std::make_shared<Func>();
	}
	/*else if (v == ValueType::STRING)
	{
		as = std::make_shared<String>();
	}*/
}
ValueContainer& ValueContainer::operator = (const ValueContainer& v)
{
	if (this == &v) return *this;
	type = v.type;
	as = v.as;
	//if (v.type == ValueType::STRING )
	//{
	//
	//	as = std::make_shared<String>(v.AsString());
	//}
	//else as = v.as;
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
		os << str;
		break;
	}
	case  ValueType::FUNCTION:
	{
		auto name =v.AsFunc()->name;
		os << name;
		break;
	}
	default:
		break;
	}
	return os;

}

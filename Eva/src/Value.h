#pragma once
#include<memory>
#include <variant>
#include <cassert>
#include<ostream>
#include"Object.h"
#include"String.hpp"

// could be a template?
enum class ValueType
{
	FLOAT,
	INT,
	BOOL,

	STRING,
	FUNCTION,
	
	DEDUCE,
	NIL
};


const char* ValueToStr(ValueType valueType);

class ValueContainer
{
public:
	ValueType type{ ValueType::NIL };
	ValueContainer() = default;
	explicit ValueContainer(ValueType v)
	{
		type = v;
	}
	template <typename T>
	ValueContainer(T v)
	{
		if constexpr (std::is_same_v<T, int>) {
			type = ValueType::INT;
		}
		else if constexpr (std::is_same_v<T, float>) {
			type = ValueType::FLOAT;
		}
		else if constexpr (std::is_same_v<T, String*>) {
			type = ValueType::STRING;
		}
		else if constexpr (std::is_same_v<T, bool>) {
			type = ValueType::BOOL;
		}
		else {
			//assert(false);
			type = ValueType::NIL;
		}
		as = v;
	}
	ValueContainer(const ValueContainer& v)
	{
		type = v.type;
		as = v.as;
		if (v.type == ValueType::STRING)
		{
			as = new String(*static_cast<String*>(std::get<String*>(v.as)));
		}
	};

	ValueContainer& operator = (bool v)
	{
		type = ValueType::BOOL;
		as = v;
		return *this;
	}

	

	ValueContainer& operator = (const ValueContainer& v)
	{
		if (this == &v) return *this;
		type = v.type;
		as = v.as;
		if (v.type == ValueType::STRING && std::get<String*>(v.as)!= nullptr)
		{
			
			as = new String(*dynamic_cast<String*>(std::get<String*>(v.as)));
		}
	}

	explicit ValueContainer(ValueContainer&& v)
	{
		type = v.type;
		as = std::move(v.as);
	}
	ValueContainer& operator = (ValueContainer&& v)
	{
		if (&v == this) return *this;
		type = v.type;
		as = std::move(v.as);

	}

	template <typename T>
	T As() const 
	{
		return std::get<T>(as);
	}
	template <typename T, typename U = T&>
	U AsRef() {
		return std::get<T>(as);
	}

private:
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class VirtualMachine;

	std::variant<bool, float, int, String*>as;
};
inline std::ostream& operator<<(std::ostream& os, const ValueContainer& v)
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
		default:
			break;
	}
	return os;
	
}

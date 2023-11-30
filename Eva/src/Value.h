#pragma once
#include<memory>
#include<ostream>
#include"Object.h"
#include"String.hpp"
// could be a template?
enum class ValueType
{
	FLOAT,
	INT,
	BOOL,
	OBJ,
	NIL
};





class ValueContainer
{
public:
	ValueType type{ ValueType::NIL };
	ValueContainer() = default;
	template<typename T>
	explicit ValueContainer(const T& v)
	{
		type = ValueTypeToEnum<T>::value;
		ValueTypeToEnum<T>::SetField(as, v);
	}
	ValueContainer(const ValueContainer& v)
	{
		type = v.type;
		as = v.as;
		if (v.type == ValueType::OBJ)
		{
			as.object = new String(*static_cast<String*>(v.as.object));
		}
	};

	ValueContainer& operator = (bool v)
	{
		type = ValueType::BOOL;
		as.boolean = v;
		return *this;
	}

	

	ValueContainer& operator = (const ValueContainer& v)
	{
		if (this == &v) return *this;
		type = v.type;
		as = v.as;
		if (v.type == ValueType::OBJ)
		{
			
			as.object = new String(*dynamic_cast<String*>(v.as.object));
		}
	}

	explicit ValueContainer(ValueContainer&& v)
	{
		type = v.type;
		as = std::move(v.as);
		v.as.object = nullptr;
	}
	ValueContainer& operator = (ValueContainer&& v)
	{
		if (&v == this) return *this;
		type = v.type;
		as = std::move(v.as);
		v.as.object = nullptr;

	}


	

	template <typename T>
	T As()
	{
		auto obj = reinterpret_cast<const Object*>(&as);
		return *reinterpret_cast<const T*>(&as);
	}
private:
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class VirtualMachine;

	union ValueAs
	{
		bool boolean{ false };
		float numberFloat;
		int numberInt;
		Object* object;
	}as;

	template <typename T>
	struct ValueTypeToEnum
	{
		static void SetField(ValueAs& as, const T& value);
	};

	template <>
	struct ValueTypeToEnum<bool> {
		static const ValueType value = ValueType::BOOL;
		
		static void SetField(ValueAs& as, bool value)
		{
			as.boolean = value;
		}
	};

	template <>
	struct ValueTypeToEnum<int> {
		static const ValueType value = ValueType::INT;
		static void SetField(ValueAs& as, int value)
		{
			as.numberInt = value;
		}
	};

	template <>
	struct ValueTypeToEnum<float> {
		static const ValueType value = ValueType::FLOAT;
		static void SetField(ValueAs& as, float value)
		{
			as.numberFloat = value;
		}
	};

	template <>
	struct ValueTypeToEnum<Object*> {
		static const ValueType value = ValueType::OBJ;
		static void SetField(ValueAs& as, Object* value)
		{
			as.object = value;
		}
	};
	


	
	
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
		case  ValueType::INT:
		{
			int num = v.as.numberInt;
			os << num;
			break;
		}
		case  ValueType::OBJ:
		{
			auto str = static_cast<String*>(v.as.object);
			os << *str;
			break;
		}
		default:
			break;
	}
	return os;
	
}
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
	template <typename T>
	T As()
	{
		return *reinterpret_cast<const T*>(&as);
	}
private:
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class VirtualMachine;
	union ValueAs
	{
		bool boolean{ false };
		float numberFloat;
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
		default:
			break;
	}
	return os;
	
}
#pragma once
#include<memory>
#include <variant>
#include <cassert>
#include <string>
#include<ostream>
//#include"Object.h"
//#include <string>
//#include"Function.h"

// could be a template?
enum class ValueType
{
	BOOL,
	INT,
	FLOAT,

	STRING,
	FUNCTION ,
	
	DEDUCE,
	NIL
};
inline ValueType HighestType(ValueType a, ValueType b) {
	if (a > b) return a;
	return b;
}

inline bool IsCastable(ValueType toType, ValueType fromType)
{
	if (toType == fromType)
		return true;

	switch (toType)
	{
	case ValueType::INT:
		return fromType == ValueType::FLOAT ||
			fromType == ValueType::BOOL ||
			fromType == ValueType::INT;

	case ValueType::FLOAT:
		return fromType == ValueType::INT ||
			fromType == ValueType::FLOAT;

	case ValueType::STRING:
		return fromType == ValueType::STRING;

	case ValueType::BOOL:
		return fromType == ValueType::INT ||
			fromType == ValueType::FLOAT ||
			fromType == ValueType::BOOL;

	case ValueType::NIL:
		
		return fromType == ValueType::NIL;

		
	default:
		return false;
	}
}

class Object;
class String;
struct Func;
class VirtualMachine;
const char* ValueToStr(ValueType valueType);
class ValueContainer
{
public:
	ValueType type{ ValueType::NIL };
	ValueContainer() = default;
	explicit ValueContainer(ValueType v);
	template <typename T>
	ValueContainer(T v)
	{
		if constexpr (std::is_same_v<T, int>) {
			type = ValueType::INT;
		}
		else if constexpr (std::is_same_v<T, float>) {
			type = ValueType::FLOAT;
		}
		else if constexpr (std::is_same_v<T, std::string> 
			|| std::is_same_v<T, const char*>) {
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
	ValueContainer(const ValueContainer& v);
	

	ValueContainer& operator = (bool v)
	{
		type = ValueType::BOOL;
		as = v;
		return *this;
	}

	void UpdateType(ValueType type);

	ValueContainer& operator = (const ValueContainer& v);

	static ValueContainer Add(const ValueContainer& v1, const ValueContainer& v2, VirtualMachine& vm);
	static ValueContainer Substract(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Multiply(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Divide(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer And(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Or(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Greater(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Less(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Equal (const ValueContainer& v1, const ValueContainer& v2);


	void Negate();

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
	std::string AsString() const
	{
		return std::get<std::string>(as);
	}
	std::shared_ptr<Func> AsFunc() const
	{
		return std::get<std::shared_ptr<Func>>(as);
	}
private:
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class VirtualMachine;

	std::variant<bool, float, int, std::string,std::shared_ptr<Func>> as;
};

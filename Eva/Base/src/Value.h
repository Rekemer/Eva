#pragma once
#include <cereal/access.hpp>
#include<memory>
#include <variant>
#include <cassert>
#include <string>
#include<ostream>
//#include"ICallable.h"
//#include <string>
//#include"Function.h"

class ValueContainer;
namespace cereal {
	template <class Archive>
	void serialize(Archive& archive, ::ValueContainer& func);
}

// could be a template?
enum class ValueType
{
	BOOL,
	INT,
	LONG,
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
			fromType == ValueType::INT ||
			fromType == ValueType::LONG;

	case ValueType::FLOAT:
		return fromType == ValueType::INT ||
			fromType == ValueType::FLOAT || 
			fromType == ValueType::LONG;

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

class ICallable;
class String;
struct Func;
class VirtualMachine;
const char* ValueToStr(ValueType valueType);

using ebool = bool;
using eint = long;
using efloat = double;
using estring = std::string;
using eCallable = std::shared_ptr<ICallable>;

using AsType = std::variant<ebool, efloat, eint, estring, eCallable>;
class ValueContainer
{
public:
	ValueType type{ ValueType::NIL };
	ValueContainer() = default;
	explicit ValueContainer(ValueType v);
	template <typename T>
	ValueContainer(T v)
	{
		if constexpr (std::is_same_v<T, eint>) {
			type = ValueType::INT;
		}
		else if constexpr (std::is_same_v<T, efloat>) {
			type = ValueType::FLOAT;
		}
		else if constexpr (std::is_same_v<T, estring> 
			|| std::is_same_v<T, const char*>) {
			type = ValueType::STRING;
		}
		else if constexpr (std::is_same_v<T, ebool>) {
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

	static ValueContainer Add(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Substract(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Multiply(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Divide(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer And(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Or(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Greater(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Less(const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Equal (const ValueContainer& v1, const ValueContainer& v2);
	static ValueContainer Percent (const ValueContainer& v1, const ValueContainer& v2);


	void Negate();
	void InverseBool();
	std::string ToString();
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

	bool operator == (const ValueContainer& v) const
	{
		if (&v == this) return true;
		return Equal(*this, v).As<ebool>();
	}
	bool operator != (const ValueContainer& v) const 
	{
		if (&v == this) return false;
		return !Equal(*this, v).As<ebool>();

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
	estring AsString() const
	{
		return std::get<estring>(as);
	}
	std::shared_ptr<Func> AsFunc() const;
	eCallable AsCallable() const;
	
private:
	friend class cereal::access;
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class  VirtualMachine;
	 template <class Archive>
    friend void cereal::serialize(Archive& archive, ValueContainer& v);
	AsType  as;
};

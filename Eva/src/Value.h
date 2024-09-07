#pragma once
#include<memory>
#include <variant>
#include <cassert>
#include<ostream>
//#include"Object.h"
//#include"String.hpp"
//#include"Function.h"

// could be a template?
enum class ValueType
{
	FLOAT,
	INT,
	BOOL,

	STRING,
	FUNCTION ,
	
	DEDUCE,
	NIL
};

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
		else if constexpr (std::is_same_v<T, std::shared_ptr<String>> ||
			std::is_same_v<T, std::shared_ptr<const String>>) {
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
	std::shared_ptr<String> AsString() const
	{
		return std::get<std::shared_ptr<String>>(as);
	}
	std::shared_ptr<Func> AsFunc() const
	{
		return std::get<std::shared_ptr<Func>>(as);
	}
private:
	friend std::ostream& operator<<(std::ostream& os, const ValueContainer& v);
	friend class VirtualMachine;

	std::variant<bool, float, int, std::shared_ptr<String>,std::shared_ptr<Func>> as;
};

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Doctest.h"
#include "VirtualMachine.h"
#include "Compile.h"
#include "Value.h"
#include "HashTable.h"



TEST_CASE("testing bool expressions")
{
	CHECK(Compile("1 == 1").As<bool>() == true);
	CHECK(Compile("1 == (1+2)").As<bool>() == false);
	CHECK(Compile("(1-2) == (1+2)").As<bool>() == false);
	CHECK(Compile("true == true").As<bool>() == true);
	CHECK(Compile("false == true").As<bool>() == false);
	CHECK(Compile("1/2 == 1/2").As<bool>() == true);
	CHECK(Compile("(2*10 - 1/5) == (2*10 - 1/5) ").As<bool>() == true);
	CHECK(Compile("2*10  == 2*10 ").As<bool>() == true);
	CHECK(Compile("2+2 ").As<int>() == 4);
	CHECK(Compile("true == true").As<bool>() == true);
	CHECK(Compile("false&&true == true&&true").As<bool>() == false);
	CHECK(Compile("false&&true || true&&true").As<bool>() == true);
	CHECK(Compile("false&&true || true&&false").As<bool>() == false);
	CHECK(Compile("false&&true").As<bool>() == false);
	CHECK(Compile("true&&true").As<bool>() == true);
	CHECK(Compile("true || false").As<bool>() == true);
	CHECK(Compile("-2 == -2+1-1 &&  1 == 1").As<bool>() == true);
	CHECK(Compile("2+2 == 2+1+1 ").As<bool>() == true);
}
TEST_CASE("testing strings ")
{
	CHECK(Compile("\"Helo\" == \"Hello\" ").As<bool>() == false);
	CHECK(Compile("\"Hello\" == \"Hello\" ").As<bool>() == true);
	CHECK(Compile("\"Hello\" == \"Hello\" ").As<bool>() == true);
	CHECK(Compile("\"2\" == \"2\" ").As<bool>() == true);
	CHECK(Compile("\"Hello\" == \"Hello\" &&  \"Hello\" == \"Hello\"").As<bool>() == true);
}
TEST_CASE("testing hash table ")
{
	HashTable table;
	table.Add( "hi",ValueContainer(2.f));
	auto entry = table.Get("hi");
	CHECK(entry->value.As<float>() == 2.f);
	CHECK(table.IsExist("hi") == true);
	table.Delete("hi");
	CHECK(table.IsExist("hi") == false);

}
struct Tables
{
	HashTable globals;
	HashTable globalTypes;
};
bool AllTrue(bool condition) {
	return condition;
}

template<typename... T>
bool AllTrue(bool condition,T... conditions)
{
	return condition && AllTrue(conditions...);
}
template <typename ExpectedType>
bool CheckVariable(std::string_view variableName, const ExpectedType& expectedValue, ValueType expectedValueType,VirtualMachine& vm)
{
	Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
	auto entry = tb.globals.Get(variableName);
	auto entryType = tb.globalTypes.Get(variableName);
	auto isType = entryType->value.type == expectedValueType;
	auto isRightValue = entry->value.As<ExpectedType>() == expectedValue;
	auto isPass = AllTrue(isType, isRightValue);
	return isPass;
}

template <>
bool CheckVariable<String>(std::string_view variableName, const String& expectedValue, ValueType expectedValueType, VirtualMachine& vm)
{
	Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
	auto entry = tb.globals.Get(variableName);
	auto entryType = tb.globalTypes.Get(variableName);
	auto isType = entryType->value.type == expectedValueType;
	auto isRightValue = entry->value.As<String&>() == expectedValue;
	auto isPass = AllTrue(isType, isRightValue);
	return isPass;
}

TEST_CASE("variable declared and has values")
{
	

	SUBCASE("declare int")
	{
		auto str = R"(a : int = 2;)";
		auto vm = CompileRetVM(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto isPass = CheckVariable<int>("a", 2, ValueType::INT,vm);
		CHECK(isPass);
	}
	SUBCASE("declare float")
	{
		auto str = R"(a : float = 45.00;)";
		auto vm = CompileRetVM(str);
		auto isPass = CheckVariable<float>("a", 45.00, ValueType::FLOAT,  vm);
		CHECK(isPass);
	}
	SUBCASE("declare bool")
	{
		auto str = R"(a : bool = true;)";
		auto vm = CompileRetVM(str);
		auto isPass = CheckVariable<bool>("a", true, ValueType::BOOL, vm);
		CHECK(isPass);
	}
	SUBCASE("declare String")
	{
		auto str = R"(a : String = "Hello, New Year!";)";
		auto vm = CompileRetVM(str);
		//auto str = String("Hello, New Year!");
		auto isPass = CheckVariable<String>("a", "Hello, New Year!", ValueType::STRING, vm);
		//Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		//auto entry = tb.globals.Get("a");
		//auto entryType = tb.globalTypes.Get("a");
		//auto isType = entryType->value.type == ValueType::STRING;
		//auto isRightValue = entry->value.As<ExpectedType>() == expectedValue;
		//auto isPass = AllTrue(isType, isRightValue);
		//return isPass;
		CHECK(isPass);
	}
}


TEST_CASE("variable declared and cast value to type")
{
	SUBCASE("cast float to int")
	{
		auto floatToInt = R"(intValue : int = 2.0;)";
		auto vm = CompileRetVM(floatToInt);
		auto isPass = CheckVariable<int>("intValue",2, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("cast int to float ")
	{
		auto intToFloat = R"(floatValue : float = 2;)";
		auto vm = CompileRetVM(intToFloat);
		auto isPass = CheckVariable<float>("floatValue",2.0, ValueType::FLOAT, vm);
		CHECK(isPass);

	}
	

}
TEST_CASE("equal operations on variables")
{
	SUBCASE("+=")
	{
		auto a = R"(a: float = 2;
					a+=2;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 4.0, ValueType::FLOAT, vm);
		CHECK(isPass);

	}
	SUBCASE("-=")
	{
		auto a = R"(a: float = 100;
					a-=2;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 98.0, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("/=")
	{
		auto a = R"(a: float = 100;
					a /=2;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 50.0, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("*=")
	{
		auto a = R"(a: float = 100;
					a *=2;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 200.0, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
}
TEST_CASE("unary double operations on variables")
{
	SUBCASE("++")
	{
		auto a = R"(a: float = 100;
					a++;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 101, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("++ in expression")
	{
		auto a = R"(a: float = 100;
					a = a++ * 2 + 1;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 202 + 1, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("--")
	{
		auto a = R"(a: float = 100;
					a--;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 99, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("-- in expression")
	{
		auto a = R"(a: float = 101;
					a = a-- * 2 + 1;)";
		auto vm = CompileRetVM(a);
		auto isPass = CheckVariable<float>("a", 200 + 1, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
}




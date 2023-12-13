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
	CHECK(Compile("2 == 2 &&  1 == 1").As<bool>() == true);
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
TEST_CASE("variable declared and has values")
{
	

	SUBCASE("declare int")
	{
		auto str = R"(a : int = 2;)";
		auto vm = CompileRetVM(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("a");
		auto entryType = tb.globalTypes.Get("a");
		auto isType = entryType->value.type == ValueType::INT;
		auto isRightValue = entry->value.As<int>() == 2;
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);
	}
	SUBCASE("declare float")
	{
		auto str = R"(a : float = 45.02;)";
		auto vm = CompileRetVM(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("a");
		auto entryType = tb.globalTypes.Get("a");
		auto isType = entryType->value.type == ValueType::FLOAT;
		auto isRightValue = entry->value.As<float>() - 45.02  < 0.002 ;
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);
	}
	SUBCASE("declare bool")
	{
		auto str = R"(a : bool = true;)";
		auto vm = CompileRetVM(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("a");
		auto entryType = tb.globalTypes.Get("a");
		auto isType = entryType->value.type == ValueType::BOOL;
		auto isRightValue = entry->value.As<bool>() == true;
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);
	}
	SUBCASE("declare String")
	{
		auto str = R"(a : String = "Hello, New Year!";)";
		auto vm = CompileRetVM(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("a");
		auto entryType = tb.globalTypes.Get("a");
		auto isType = entryType->value.type == ValueType::STRING;
		auto string = (String*)(entry->value.As<Object*>());
		auto isRightValue = *string == "Hello, New Year!";
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);
	}
}

TEST_CASE("variable declared and cast value to type")
{
	SUBCASE("cast float to int")
	{
		auto floatToInt = R"(intValue : int = 2.0;)";
		auto vm = CompileRetVM(floatToInt);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("intValue");
		auto entryType = tb.globalTypes.Get("intValue");
		auto isType = entryType->value.type == ValueType::INT;
		auto isRightValue = entry->value.As<int>() == 2;
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);
	}
	SUBCASE("cast int to float ")
	{
		auto intToFloat = R"(floatValue : float = 2;)";
		auto vm = CompileRetVM(intToFloat);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto entry = tb.globals.Get("floatValue");
		auto entryType = tb.globalTypes.Get("floatValue");
		auto isType = entryType->value.type == ValueType::FLOAT;
		auto isRightValue = entry->value.As<float>() == 2.0;
		auto isPass = AllTrue(isType, isRightValue);
		CHECK(isPass);

	}
	

}


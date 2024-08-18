#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Doctest.h"
#include "VirtualMachine.h"
#include "Compile.h"
#include "Value.h"
#include "HashTable.h"

#define EXPR 1
#define LOOPS 1
#define FUNC 1
#define VAR 1


struct Tables
{
	HashTable globals;
	HashTable globalTypes;
};
bool AllTrue(bool condition) {
	return condition;
}

template<typename... T>
bool AllTrue(bool condition, T... conditions)
{
	return condition && AllTrue(conditions...);
}
template <typename ExpectedType>
bool CheckVariable(std::string_view variableName, ExpectedType expectedValue,
	ValueType expectedValueType, VirtualMachine& vm)
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
bool CheckVariable<String*>(std::string_view variableName, String* expectedValue,
	ValueType expectedValueType, VirtualMachine& vm)
{
	Tables tb = {vm.GetGlobals(), vm.GetGlobalsType() };

	auto entry = tb.globals.Get(variableName);
	auto entryType = tb.globalTypes.Get(variableName);
	auto isType = entryType->value.type == expectedValueType;
	auto isRightValue = *entry->value.AsString() == *expectedValue;
	auto isPass = AllTrue(isType, isRightValue);
	
	return true;
}

#if EXPR
	TEST_CASE("testing bool expressions")
	{
		CHECK(CompileTest("1/2 == 1/2").As<bool>() == true);
		CHECK(CompileTest("1 == 1").As<bool>() == true);
		CHECK(CompileTest("1 == (1+2)").As<bool>() == false);
		CHECK(CompileTest("(1-2) == (1+2)").As<bool>() == false);
		CHECK(CompileTest("true == true").As<bool>() == true);
		CHECK(CompileTest("false == true").As<bool>() == false);
		CHECK(CompileTest("(2*10 - 1/5) == (2*10 - 1/5) ").As<bool>() == true);
		CHECK(CompileTest("2*10  == 2*10 ").As<bool>() == true);
		CHECK(CompileTest("2+2 ").As<int>() == 4);
		CHECK(CompileTest("3%2 ").As<int>() == 1);
		CHECK(CompileTest("true == true").As<bool>() == true);
		CHECK(CompileTest("false&&true == true&&true").As<bool>() == false);
		CHECK(CompileTest("false&&true || true&&true").As<bool>() == true);
		CHECK(CompileTest("false&&true || true&&false").As<bool>() == false);
		CHECK(CompileTest("false&&true").As<bool>() == false);
		CHECK(CompileTest("true&&true").As<bool>() == true);
		CHECK(CompileTest("true || false").As<bool>() == true);
		CHECK(CompileTest("-2 == -2+1-1 &&  1 == 1").As<bool>() == true);
		CHECK(CompileTest("2+2 == 2+1+1 ").As<bool>() == true);
		CHECK(CompileTest("2+2 < 2+1+1+1 ").As<bool>() == true);
		CHECK(CompileTest("2+2 <= 2+2 ").As<bool>() == true);
		CHECK(CompileTest("2+2 >= 2+2 ").As<bool>() == true);
		CHECK(CompileTest("2+5 >= 2+2 ").As<bool>() == true);
		CHECK(CompileTest("2+5 != 2+2 ").As<bool>() == true);
		CHECK(CompileTest("!(2+5 == 2+2 )").As<bool>() == true);
	}

#endif // EXPR


TEST_CASE("testing strings ")
{
	CHECK(CompileTest("\"Helo\" == \"Hello\" ").As<bool>() == false);
	CHECK(CompileTest("\"Hello\" == \"Hello\" ").As<bool>() == true);
	CHECK(CompileTest("\"Hello\" == \"Hello\" ").As<bool>() == true);
	CHECK(CompileTest("\"2\" == \"2\" ").As<bool>() == true);
	CHECK(CompileTest("\"Hello\" == \"Hello\" &&  \"Hello\" == \"Hello\"").As<bool>() == true);
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

#if VAR
TEST_CASE("variable declared and has values")
{


	SUBCASE("declare int")
	{
		auto str = R"(a : int = 2;)";
		auto [res, vm] = Compile(str);
		Tables tb = { vm.GetGlobals() ,vm.GetGlobalsType() };
		auto isPass = CheckVariable<int>("a", 2, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("declare float")
	{
		auto str = R"(a : float = 45.00;)";
		auto [res, vm] = Compile(str);
		auto isPass = CheckVariable<float>("a", 45.00, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("declare bool")
	{
		auto str = R"(a : bool = true;)";
		auto [res, vm] = Compile(str);
		auto isPass = CheckVariable<bool>("a", true, ValueType::BOOL, vm);
		CHECK(isPass);
	}
	SUBCASE("declare String")
	{
		auto str = R"(a : String = "Hello, New Year!";)";
		auto [res, vm] = Compile(str);
		String checkString = "Hello, New Year!";
		auto isPass = CheckVariable<String*>("a", &checkString, ValueType::STRING, vm);
		CHECK(isPass);
	}
}


TEST_CASE("variable declared and cast value to type")
{
	SUBCASE("cast float to int")
	{
		auto floatToInt = R"(intValue : int = 2.0;)";
		auto [res, vm] = Compile(floatToInt);
		auto isPass = CheckVariable<int>("intValue", 2, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("cast int to float ")
	{
		auto intToFloat = R"(floatValue : float = 2;)";
		auto [res, vm] = Compile(intToFloat);
		auto isPass = CheckVariable<float>("floatValue", 2.0, ValueType::FLOAT, vm);
		CHECK(isPass);

	}


}
TEST_CASE("equal operations on variables")
{
	SUBCASE("+=")
	{
		auto a = R"(a: float = 2;
					a+=2;)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", 4.0, ValueType::FLOAT, vm);
		CHECK(isPass);

	}
	SUBCASE("-=")
	{
		auto a = R"(a: float = 100;
					a-=2;)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", 98.0, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("/=")
	{
		auto a = R"(a: float = 100;
					a /=2;)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", 50.0, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("*=")
	{
		auto a = R"(a: float = 100;
					a *=2;)";
		auto [res, vm] = Compile(a);
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
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", 101, ValueType::FLOAT, vm);
		CHECK(isPass);
	}

	SUBCASE("--")
	{
		auto a = R"(a: float = 100;
					a--;)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", 99, ValueType::FLOAT, vm);
		CHECK(isPass);
	}

	/*SUBCASE("++ in expression")
	{
		auto a = R"(a: float = 100;
					a = a++ * 2 + 1;)";
		auto [res,vm]= Compile(a);
		auto isPass = CheckVariable<float>("a", 202 + 1, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	SUBCASE("-- in expression")
	{
		auto a = R"(a: float = 101;
					a = a-- * 2 + 1;)";
		auto [res,vm]= Compile(a);
		auto isPass = CheckVariable<float>("a", 200 + 1, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
	*/
}
#endif



TEST_CASE("scope test")
{
	SUBCASE("change global variable in local context")
	{
		auto a = R"(a: int = 15;
				{
					a--;
				}
					)";
		auto [res,vm]= Compile(a);
		auto isPass = CheckVariable<int>("a", 14, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("change global variable via local variable")
	{
		auto a = R"(a: int = 3;
		{
			b : int = 15;
			b+=2;
			b-=2;
			b/=5;
			b*=4.0;
			a = b;
		}

					)";
		auto [res,vm]= Compile(a);
		auto isPass = CheckVariable<int>("a", 12, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("multiple scopes")
	{
		auto a = R"(a: int = 3;
		{
			b : int = 15;
			b+=2;
			{
				c : float = -1;
				b*= c;
				a -=c;
			}
			a+=b;
		}

					)";
		auto [res,vm]= Compile(a);
		auto isPass = CheckVariable<int>("a", -13, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("multiple scopes in a row")
	{
		auto a = R"(a: float = 3;
			{
				a: float = 2;
				a+=2;
			}
			
			{
				b : int = 15;
				b+=2;
				{
					c : float = -1;
					b*= c;
					a -=c;
				}
				a+=b;
			}
			{
				b : int = 15;
				b+=2;
				{
					c : float = -1;
					b*= c;
					a -=c;
				}
				a+=b;
			}

					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("a", -29, ValueType::FLOAT, vm);
		CHECK(isPass);
	}
}
TEST_CASE("deduction test")
{
	SUBCASE("global deduction scopes")
	{
		auto a = R"(a:= 3;
					a-=2;
					b := 2.2 + 2;
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 1, ValueType::INT, vm) && 
			CheckVariable<float>("b", 4.2, ValueType::FLOAT, vm);;
		CHECK(isPass);
	}
	SUBCASE("local deduction scopes")
	{
		auto a = R"(
					g := 0.0;
					{
						a:= 3;
						a-=2;
						b := 2.2 + 2;
						g = a + b;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<float>("g", 5.2, ValueType::FLOAT, vm);
		CHECK(isPass);
	}

}
#if LOOPS
TEST_CASE("while statement")
{
	SUBCASE("basic while")
	{
		auto a = R"(a: int = 15;
						while a  >= 5 
					{
						a--;
					}
						)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 4, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("while loop continue")
	{
		auto a = R"(
					g := 0;
					a := 0;
					while a < 5
					{	
						a++;
						if (a % 2 == 1) == true
						{
							continue;
						}
						g+=a;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (2 + 4), ValueType::INT, vm);
		CHECK(isPass);
	}

	SUBCASE("while loop break")
	{
		auto a = R"(
					g := 0;
					a := 0;
					while a < 5
					{	
						a++;
						if a == 4 
						{
							break;
						}
						g+=a;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (1 + 2 + 3), ValueType::INT, vm);
		CHECK(isPass);
	}
}
TEST_CASE("basic for loop test")
{
	SUBCASE("basic for loop")
	{
		auto a = R"(
					g := 0;
					for  i:= 0; i < 10; i+=1;
					{
						g+=i;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 45, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("basic for loop continue")
	{
		auto a = R"(
					g := 0;
					for i:=1; i < 5; i++;
					{	
						if i % 2 == 1
						{
							continue;
						}
						g+=i;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (2 + 4), ValueType::INT, vm);
		CHECK(isPass);
	}

	SUBCASE("basic for loop break")
	{
		auto a = R"(
					g := 0;
					for i:=1; i < 5; i++;
					{	
						if i == 4 
						{
							break;
						}
						g+=i;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (1 + 2 + 3), ValueType::INT, vm);
		CHECK(isPass);
	}
}
TEST_CASE("folded for loop test")
{

	SUBCASE("folded constant for loop")
	{
		auto a = R"(
				g := 0;
				for i:=  2..5
				{
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 9, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("folded end constant for loop")
	{
		auto a = R"(
				g := 0;
				a := 2;
				for i:= a..5
				{
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 9, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("folded start constant for loop")
	{
		auto a = R"(
				g := 0;
				a := 5;
				for i:= 2..a
				{
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 9, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("folded variable for loop")
	{
		auto a = R"(
				g := 0;
				a := 5;
				c := 2;
				for i:= c..a
				{
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 9, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("folded for loop continue")
	{
		auto a = R"(
				g := 0;
				for i:= 1..5
				{	
					if (i % 2 == 1) 
					{
						continue;
					}
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (2 + 4), ValueType::INT, vm);
	}

	SUBCASE("folded for loop break")
	{
		auto a = R"(
				g := 0;
				for i:= 1..5
				{	
					if i == 4 
					{
						break;
					}
					g+=i;
				}
				)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", (1 + 2 + 3), ValueType::INT, vm);
		CHECK(isPass);
	}
}

TEST_CASE("loops in loops")
{
	SUBCASE("for loop in for")
	{
		auto a = R"(
					a:=0;
					for i:= 0..5
					{
						if i % 2 == 1 
						{
							continue;
						}
						a+=i;
						for j:=0; j < 5; j++; 
						{
							if j == 4
							{
								break;
							}
							a+=j;
						}
					}
					)";
		auto [res, vm] = Compile(a);
		auto sumJPerI = 1 + 2 + 3;
		auto isPass = CheckVariable<int>("a", (sumJPerI * 3 + 2 + 4), ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("whlie loop in while")
	{
		auto a = R"(
					a:=0;
					i:= 0;
					j:= 0;
					while i < 5
					{
						i++;
						if i % 2 == 1 
						{
							continue;
						}
						a+=2;
						while j < 2
						{
							j++;
							a+=2;
						}
						j =0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", (2 * 3 + 6), ValueType::INT, vm);
		CHECK(isPass);
	}

}

#endif

TEST_CASE("if statTment")
{
	SUBCASE("if")
	{
		auto a = R"(a: int = 100;
					if a == 100
					{
						a++;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 101, ValueType::INT, vm);
		CHECK(isPass);
	}

	SUBCASE("if else")
	{
		auto a = R"(a: int = 100;
					if a == 101
					{
						a++;
					}
					else 
					{
						a+=5;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 105, ValueType::INT, vm);
		CHECK(isPass);
	}

	SUBCASE("elif")
	{
		auto a = R"(a: int = 105;
					if a == 101
					{
						a++;
					}
					elif a == 102 
					{
						a+=5;
					}
					elif a == 105 {
					a +=10;		
					} 
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 115, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE(" if elif else")
	{
		auto a = R"(a: int = 105;
					if a == 101
					{
						a++;
					}
					elif a == 102 
					{
						a+=5;
					}
					elif a == 103 {
						a +=10;		
					} 
					else
					{
						a +=20;		
					} 
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 125, ValueType::INT, vm);
		CHECK(isPass);
	}
}
#if FUNC
TEST_CASE("functions")
{
	SUBCASE("call getting global resuls")
	{
		auto a = R"(	
					a := 5+2;
					fun main() : int
					{
						a++;
						return 0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 8, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("main calling functions")
	{
		auto a = R"(
					fun mult (a : int, b: int) : int
					{
						return a*b;
					}
					fun foo( a: int, b : int ) : int 
					{ 
						return mult(a,b) + mult(a,b);
					}
					a := 0;
					fun main() : int
					{
						a = foo(2,2);
						return 0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 8, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("initialize variable via function ")
	{
		auto a = R"(
					fun num() : int
					{
					   return 2;
					}
					g: int = 0;
					fun main() : int
					{
						a := num();
						g = a;
						return 0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 2, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("fibbonaci")
	{
		auto a = R"(
					fun fib(n : int) : int
					{
						if n < 2 
						{
							return n;
						}
						return fib(n - 2) + fib(n - 1);
					}
					g: int = 0;
					fun main() : int
					{
						a := fib(10);
						g = a;
						return 0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 55, ValueType::INT, vm);
		CHECK(isPass);
	}
}
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Doctest.h"
#include "VirtualMachine.h"
#include "Compile.h"
#include "Value.h"
#include "HashTable.h"
#include <format>


#define EXPR 0
#define WHILE 0
#define FOR 0
#define FUNC 0
#define VAR 0
#define STRINGS 0
#define SCOPE 0
#define DEDUCTION 0
#define IF 1
#define CONSTANT_FOLD 0

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
bool CheckVariable(const std::string& variableName, ExpectedType expectedValue,
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
		//
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
		//
		CHECK(CompileTest("2.0 >= 2 ").As<bool>() == true);
		CHECK(CompileTest("2.0 == 2 ").As<bool>() == true);
		CHECK(CompileTest("2.0 != 2 ").As<bool>() == false);
		CHECK(CompileTest("2.0 < 4.0 ").As<bool>() == true);
		CHECK(CompileTest("2 <= 4.0 ").As<bool>() == true);
		////
		CHECK(CompileTest("2+5 >= 2+2 ").As<bool>() == true);
		CHECK(CompileTest("2+5 != 2+2 ").As<bool>() == true);
		CHECK(CompileTest("!(2+5 == 2+2 )").As<bool>() == true);
		CHECK(CompileTest("(2/2 * 2 == 2 )").As<bool>() == true);
		CHECK(CompileTest("(4* 6/3 == 8 )").As<bool>() == true);
		CHECK(CompileTest("(4.0* 6/3 == 8.0 )").As<bool>() == true);
		CHECK(CompileTest("(4.0* 6/3 + 2 == 10.0 )").As<bool>() == true);
		CHECK(CompileTest("(4.0* 6/(4/2.0) + 2 == 14.0 )").As<bool>() == true);
		CHECK(CompileTest("( 6.0/(6/2.0) * 4.0 + 2 == 10.0 )").As<bool>() ==true);
		CHECK(CompileTest("( 2.0 / 4 * 3  == 1.5)").As<bool>() == true);
		CHECK(CompileTest("( 3 * 2 / 4.0 == 6.0/4)").As<bool>() == true);
	}

#endif // EXPR

#if STRINGS
	TEST_CASE("testing strings ")
	{
		CHECK(CompileTest("\"Helo\" == \"Hello\" ").As<bool>() == false);
		CHECK(CompileTest("\"Hello\" == \"Hello\" ").As<bool>() == true);
		CHECK(CompileTest("\"Hello\" == \"Hello\" ").As<bool>() == true);
		CHECK(CompileTest("\"2\" == \"2\" ").As<bool>() == true);
		CHECK(CompileTest("\"Hello\" == \"Hello\" &&  \"Hello\" == \"Hello\"").As<bool>() ==	true);
	}
#endif // STRING

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
		std::string checkString = "Hello, New Year!";
		auto isPass = CheckVariable<std::string>("a", checkString, ValueType::STRING, vm);
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


#if SCOPE

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

	SUBCASE("declare variable after scope ends")
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
			e := 2;
			
			a+=b;
			a+=e;
		}

					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", -11, ValueType::INT, vm);
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
#if SCOPE && IF
	SUBCASE("=")
	{
		auto a = R"(

		d_g :=0;
		c_g :=0;
		{
			  a: int = 1;
			  a++;
			  a = 5;
			  b:int = a;
			  c:int = 2;
			  {
				 a := 5;
				 b:int =a;
				 c:int = 2;
			  }
			   d := 6;
			  d = 2 ;
			check := false;
			if (check)
			{
			d = 5;
			}
			else 
			{
			d = 10;
			}
			 c = 5;
			d_g = d;
			c_g = c;
			
		}
)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("d_g", 10, ValueType::INT, vm) &&
			CheckVariable<int>("c_g", 5, ValueType::INT, vm);
		CHECK(isPass);

	}
#endif
	
}
#endif // 0
#if DEDUCTION
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
#endif
#if WHILE
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
#if FOR
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
		constexpr auto declRes = (1 + 2 + 3) * 3 + 2 + 4;

		auto isPass = CheckVariable<int>("a", declRes, ValueType::INT, vm);
		CHECK(isPass);
	}
}
#endif

#if IF
TEST_CASE("if statement")
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
	SUBCASE(" if elif else else taken")
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
	SUBCASE(" if elif else elif taken")
	{
		auto a = R"(a: int = 102;
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
		auto isPass = CheckVariable<int>("a", 107, ValueType::INT, vm);
		CHECK(isPass);
	}

	SUBCASE(" if elif  different conditions elif taken")
	{
		auto a = R"(a: int = 102;
					if a == 101
					{
						a++;
					}
					elif a < 102 
					{
						a+=5;
					}
					elif a > 20 {
						a +=10;		
					} 
					
					)";

		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 112, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE(" if elif else  different conditions elif taken")
	{
		auto a = R"(a: int = 102;
					if a == 101
					{
						a++;
					}
					elif a < 102 
					{
						a+=5;
					}
					elif a > 20 {
						a +=10;		
					} 
					else 
					{
						a*= 10; 
					}
					
					)";

		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 112, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("Nested if statements in ifs, elifs, and else being taken")
	{
		auto a = R"(a: int = 102;
                if a == 101
                {
                    if a > 100
                    {
                        a++;
                    }
                    else
                    {
                        a--;
                    }
                }
                elif a < 102 
                {
                    if a > 50
                    {
                        a += 5;
                    }
                    else
                    {
                        a -= 5;
                    }
                }
                elif a != 102 {
                    if a % 2 == 0
                    {
                        a +=10;     
                    }
                    else
                    {
                        a +=20;
                    }
                } 
                else 
                {
                    if a == 102
                    {
                        a *= 10; 
                    }
                    else
                    {
                        a /= 10;
                    }
                }
                
                )";

		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 1020, ValueType::INT, vm);
		CHECK(isPass);
	}

}
#endif // IF
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
	SUBCASE("global call pass constants")
	{
		auto a = R"(	
					fun sum(a : int, b : int, c : int) : int 
					{
						return a+b+c;
					}
					a := sum(10,3,4);
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a",17 , ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("global call pass variables")
	{
		auto a = R"(	
					fun sum(a : int, b : int, c : int) : int 
					{
						return a+b+c;
					}
					arg1 := 10;
					arg2 := 3;
					arg3 := 4;
					a := sum(arg1,arg2,arg3);
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 17, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("global call pass temporaries")
	{
		auto a = R"(	
					fun sum(a : int, b : int, c : int) : int 
					{
						return a+b+c;
					}
					arg1 := 9;
					arg2 := 4;
					arg3 := 3;
					a := sum(arg1 + 1,arg2 -1 ,arg3+ 1);
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("a", 17, ValueType::INT, vm);
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
	SUBCASE("pass different temp var const data types to function")
	{
		auto a = R"(
					fun sum(n : int, n1 : int, n2: int, n3:int) : int
					{
						return n + n1 + n2 + n3;
					}
					g: int = 0;
					fun main() : int
					{	
						c:=10;
						a := sum(c,10, g+ 10,g);
						g = a;
						return 0;
					}
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 30, ValueType::INT, vm);
		CHECK(isPass);
	}
	SUBCASE("call function global")
	{
		auto a = R"(
					fun sum(n : int, n1 : int, n2: int) : int
					{
						return n + n1 + n2;
					}
					g: int = sum(1,2,3);
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<int>("g", 6, ValueType::INT, vm);
		CHECK(isPass);
	}
}

#endif
#if CONSTANT_FOLD && FUNC

TEST_CASE("constant folding")
{
	SUBCASE("constant folding all cases")
	{
		auto a = R"(

					
					fun foo(n : int) : int
					{
						return n + 2;
					}
					
					a := 2;

					c0 := foo(2.0 + 3.5) + a + 3 + 3;

					c := 3 + a + 3 ; 
					c1 := 3 + 3 + a;
					c2 := 2 *6  + a - 3 - 3/2;
					c3 :=  a + 2 *6   - 3 - 3/2 - c+ c -c;
					c4 := 2 *6  + a - c;
					c5 := c * 2.0 * a / 4;
					c6 := c * 2.0 * a / 4 * a/16;
					d := 1 - 2 * 0;
					b:= 0/2 + d + 2 ;

					check : bool = c0 == 15 && d == 1 && b == 3&& c == 8 && c1 == 8  &&
					 c2 == 10 && c3 == (c2 - c + c - c) && c4 == 6  && c5 == 8.0 
					 && c6 == 1.0;   
					)";
		auto [res, vm] = Compile(a);
		auto isPass = CheckVariable<bool>("check", true, ValueType::BOOL, vm);
		CHECK(isPass);
	}
}



#endif

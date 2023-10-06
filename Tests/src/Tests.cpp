#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Doctest.h"
#include "VirtualMachine.h"
#include "Compile.h"
#include "Value.h"
int factorial(int number) { return number <= 1 ? number : factorial(number - 1) * number; }



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
CHECK(Compile("2+2 ").As<float>() == 4);
 CHECK(Compile("\"Helo\" == \"Hello\" ").As<bool>() == false);
 CHECK(Compile("\"Hello\" == \"Hello\" ").As<bool>() == true);
}

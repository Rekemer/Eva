#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Doctest.h"
#include "VirtualMachine.h"
#include "Compile.h"
#include "Value.h"
int factorial(int number) { return number <= 1 ? number : factorial(number - 1) * number; }



TEST_CASE("testing bool expressions")
{
   CHECK(Compile("1 == 1").as.boolean == true);
   CHECK(Compile("1 == (1+2)").as.boolean == false);
   CHECK(Compile("(1-2) == (1+2)").as.boolean == false);
   CHECK(Compile("true == true").as.boolean == true);
   CHECK(Compile("false == true").as.boolean == false);
}
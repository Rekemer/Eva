SOURCE_HEADER = r'''#include "glfw_wrappers.h"
#include  <stdio.h> // for demo printing
#include  <glfw3.h> // or your dynamic load approach
#include  <unordered_map>
#include  <string>
#include  "Value.h"
#include  "CallState.h"
namespace Eva {
#define EXPORT extern "C" __declspec(dllexport)
'''
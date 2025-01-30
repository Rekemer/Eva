#include "glfw_wrappers.h"
#include  <stdio.h> // for demo printing
#include  <glfw3.h> // or your dynamic load approach
#include  <vector>
#include  <unordered_map>
#include  <string>
#include  "Value.h"
namespace Eva {
// We'll define a dummy MyState to show usage:
struct MyState {
    // In real usage, you'd store a stack of values, or arguments, etc.
    // This is just a placeholder.
    std::vector<ValueContainer>* stack;
    
};

int wrapper_glfwInit(MyState* st) {
    

    auto result = glfwInit();
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwTerminate(MyState* st) {
    

    glfwTerminate();
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwCreateWindow(MyState* st) {
    
      auto arg4 = st->stack->back();
	st->stack->pop_back();
      auto arg3 = st->stack->back();
	st->stack->pop_back();
      auto arg2 = st->stack->back();
	st->stack->pop_back();
      auto arg1 = st->stack->back();
	st->stack->pop_back();
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    void* result = glfwCreateWindow(arg0.As<eint>(), arg1.As<eint>(), reinterpret_cast<const char*>(arg2.AsString().data()), reinterpret_cast<GLFWmonitor*>(arg3.As<eptr>()), reinterpret_cast<GLFWwindow*>(arg4.As<eptr>()));
    st->stack->push_back(ValueContainer(reinterpret_cast<eptr>(result)));
    return 1; // number of values we 'return'
}

int wrapper_glfwDestroyWindow(MyState* st) {
    
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwMakeContextCurrent(MyState* st) {
    
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwWindowShouldClose(MyState* st) {
    
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    auto result = glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwSetWindowShouldClose(MyState* st) {
    
      auto arg1 = st->stack->back();
	st->stack->pop_back();
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), arg1.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwPollEvents(MyState* st) {
    

    glfwPollEvents();
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwSwapBuffers(MyState* st) {
    
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwGetKey(MyState* st) {
    
      auto arg1 = st->stack->back();
	st->stack->pop_back();
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    auto result = glfwGetKey(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), arg1.As<eint>());
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwGetTime(MyState* st) {
    

    auto result = glfwGetTime();
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwWindowHint(MyState* st) {
    
      auto arg1 = st->stack->back();
	st->stack->pop_back();
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwWindowHint(arg0.As<eint>(), arg1.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwSwapInterval(MyState* st) {
    
      auto arg0 = st->stack->back();
	st->stack->pop_back();
    glfwSwapInterval(arg0.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

GLFWFuncEntry g_glfwFunctionTable[] = {
    {"glfwInit", &wrapper_glfwInit},
    {"glfwTerminate", &wrapper_glfwTerminate},
    {"glfwCreateWindow", &wrapper_glfwCreateWindow},
    {"glfwDestroyWindow", &wrapper_glfwDestroyWindow},
    {"glfwMakeContextCurrent", &wrapper_glfwMakeContextCurrent},
    {"glfwWindowShouldClose", &wrapper_glfwWindowShouldClose},
    {"glfwSetWindowShouldClose", &wrapper_glfwSetWindowShouldClose},
    {"glfwPollEvents", &wrapper_glfwPollEvents},
    {"glfwSwapBuffers", &wrapper_glfwSwapBuffers},
    {"glfwGetKey", &wrapper_glfwGetKey},
    {"glfwGetTime", &wrapper_glfwGetTime},
    {"glfwWindowHint", &wrapper_glfwWindowHint},
    {"glfwSwapInterval", &wrapper_glfwSwapInterval},
};

int g_glfwFunctionCount = sizeof(g_glfwFunctionTable)/sizeof(GLFWFuncEntry);
std::unordered_map<std::string, ValueType> initModule() {
    std::unordered_map<std::string, ValueType> typeMap;
    typeMap["glfwInit"] = ValueType::INT;
    typeMap["glfwTerminate"] = ValueType::NIL;
    typeMap["glfwCreateWindow"] = ValueType::PTR;
    typeMap["glfwDestroyWindow"] = ValueType::NIL;
    typeMap["glfwMakeContextCurrent"] = ValueType::NIL;
    typeMap["glfwWindowShouldClose"] = ValueType::INT;
    typeMap["glfwSetWindowShouldClose"] = ValueType::NIL;
    typeMap["glfwPollEvents"] = ValueType::NIL;
    typeMap["glfwSwapBuffers"] = ValueType::NIL;
    typeMap["glfwGetKey"] = ValueType::INT;
    typeMap["glfwGetTime"] = ValueType::FLOAT;
    typeMap["glfwWindowHint"] = ValueType::NIL;
    typeMap["glfwSwapInterval"] = ValueType::NIL;

    return typeMap;
}
}
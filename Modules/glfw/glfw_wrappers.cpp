#include "glfw_wrappers.h"
#include  <stdio.h> // for demo printing
#include  <GLFW/glfw3.h> // or your dynamic load approach
#include  <vector>
#include  "Value.h"

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
    
      auto arg4 = st->stack->pop_back(); // parse from st
      auto arg3 = st->stack->pop_back(); // parse from st
      auto arg2 = st->stack->pop_back(); // parse from st
      auto arg1 = st->stack->pop_back(); // parse from st
      auto arg0 = st->stack->pop_back(); // parse from st
    void* result = glfwCreateWindow(arg{i}.As<eint>(), arg{i}.As<eint>(), reinterpeter_cast<const char*>(arg{i}).AsString(), reinterpeter_cast<GLFWmonitor*>(arg{i}).As<eptr>(), reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>());
    st->stack->push_back(ValueContainer(reinterpret_cast<eptr>(result)));
    return 1; // number of values we 'return'
}

int wrapper_glfwDestroyWindow(MyState* st) {
    
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwDestroyWindow(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwMakeContextCurrent(MyState* st) {
    
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwMakeContextCurrent(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwWindowShouldClose(MyState* st) {
    
      auto arg0 = st->stack->pop_back(); // parse from st
    auto result = glfwWindowShouldClose(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>());
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwSetWindowShouldClose(MyState* st) {
    
      auto arg1 = st->stack->pop_back(); // parse from st
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwSetWindowShouldClose(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>(), arg{i}.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwPollEvents(MyState* st) {
    

    glfwPollEvents();
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwSwapBuffers(MyState* st) {
    
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwSwapBuffers(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwGetKey(MyState* st) {
    
      auto arg1 = st->stack->pop_back(); // parse from st
      auto arg0 = st->stack->pop_back(); // parse from st
    auto result = glfwGetKey(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>(), arg{i}.As<eint>());
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwSetFramebufferSizeCallback(MyState* st) {
    
      auto arg1 = st->stack->pop_back(); // parse from st
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwSetFramebufferSizeCallback(reinterpeter_cast<GLFWwindow*>(arg{i}).As<eptr>(), arg{i}.LOX);
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwGetTime(MyState* st) {
    

    auto result = glfwGetTime();
    st->stack->push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

int wrapper_glfwWindowHint(MyState* st) {
    
      auto arg1 = st->stack->pop_back(); // parse from st
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwWindowHint(arg{i}.As<eint>(), arg{i}.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

int wrapper_glfwSwapInterval(MyState* st) {
    
      auto arg0 = st->stack->pop_back(); // parse from st
    glfwSwapInterval(arg{i}.As<eint>());
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
    {"glfwSetFramebufferSizeCallback", &wrapper_glfwSetFramebufferSizeCallback},
    {"glfwGetTime", &wrapper_glfwGetTime},
    {"glfwWindowHint", &wrapper_glfwWindowHint},
    {"glfwSwapInterval", &wrapper_glfwSwapInterval},
};

int g_glfwFunctionCount = sizeof(g_glfwFunctionTable)/sizeof(GLFWFuncEntry);


#include "glfw_wrappers.h"
#include  <stdio.h> // for demo printing
#include  <glfw3.h> // or your dynamic load approach
#include  <unordered_map>
#include  <string>
#include  "Value.h"
#include  "CallState.h"
namespace Eva {
#define EXPORT extern "C" __declspec(dllexport)

 EXPORT int wrapper_glfwInit(CallState& st) {
    

    auto result = glfwInit();
    st.stack.push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

 EXPORT int wrapper_glfwTerminate(CallState& st) {
    

    glfwTerminate();
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwCreateWindow(CallState& st) {
    
      auto arg4 = st.stack.back();
	st.stack.pop_back();
      auto arg3 = st.stack.back();
	st.stack.pop_back();
      auto arg2 = st.stack.back();
	st.stack.pop_back();
      auto arg1 = st.stack.back();
	st.stack.pop_back();
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    void* result = glfwCreateWindow(arg0.As<eint>(), arg1.As<eint>(), reinterpret_cast<const char*>(arg2.AsString().data()), reinterpret_cast<GLFWmonitor*>(arg3.As<eptr>()), reinterpret_cast<GLFWwindow*>(arg4.As<eptr>()));
    st.stack.push_back(ValueContainer(reinterpret_cast<eptr>(result)));
    return 1; // number of values we 'return'
}

 EXPORT int wrapper_glfwDestroyWindow(CallState& st) {
    
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwMakeContextCurrent(CallState& st) {
    
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwWindowShouldClose(CallState& st) {
    
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    auto result = glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    st.stack.push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

 EXPORT int wrapper_glfwSetWindowShouldClose(CallState& st) {
    
      auto arg1 = st.stack.back();
	st.stack.pop_back();
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), arg1.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwPollEvents(CallState& st) {
    

    glfwPollEvents();
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwSwapBuffers(CallState& st) {
    
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwGetKey(CallState& st) {
    
      auto arg1 = st.stack.back();
	st.stack.pop_back();
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    auto result = glfwGetKey(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), arg1.As<eint>());
    st.stack.push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

 EXPORT int wrapper_glfwGetTime(CallState& st) {
    

    auto result = glfwGetTime();
    st.stack.push_back(ValueContainer(result));
    return 1; // number of values we 'return'
}

 EXPORT int wrapper_glfwWindowHint(CallState& st) {
    
      auto arg1 = st.stack.back();
	st.stack.pop_back();
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwWindowHint(arg0.As<eint>(), arg1.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT int wrapper_glfwSwapInterval(CallState& st) {
    
      auto arg0 = st.stack.back();
	st.stack.pop_back();
    glfwSwapInterval(arg0.As<eint>());
    // no return, so no results to push
    return 0; // number of values we 'return'
}

 EXPORT std::unordered_map<std::string, ValueType>* initModule() {
    std::unordered_map<std::string, ValueType>* typeMap = new std::unordered_map<std::string, ValueType>{} ;
    (*typeMap)["glfwInit"] = ValueType::INT;
    (*typeMap)["glfwTerminate"] = ValueType::NIL;
    (*typeMap)["glfwCreateWindow"] = ValueType::PTR;
    (*typeMap)["glfwDestroyWindow"] = ValueType::NIL;
    (*typeMap)["glfwMakeContextCurrent"] = ValueType::NIL;
    (*typeMap)["glfwWindowShouldClose"] = ValueType::INT;
    (*typeMap)["glfwSetWindowShouldClose"] = ValueType::NIL;
    (*typeMap)["glfwPollEvents"] = ValueType::NIL;
    (*typeMap)["glfwSwapBuffers"] = ValueType::NIL;
    (*typeMap)["glfwGetKey"] = ValueType::INT;
    (*typeMap)["glfwGetTime"] = ValueType::FLOAT;
    (*typeMap)["glfwWindowHint"] = ValueType::NIL;
    (*typeMap)["glfwSwapInterval"] = ValueType::NIL;

    return typeMap;
}std::unordered_map<std::string, std::shared_ptr<NativeFunc>> nativeCalls = {
    {"glfwInit", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwInit, ICallable::INF_ARGS, "glfwInit")},
    {"glfwTerminate", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwTerminate, ICallable::INF_ARGS, "glfwTerminate")},
    {"glfwCreateWindow", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT, ValueType::INT, ValueType::STRING, ValueType::PTR, ValueType::PTR}, wrapper_glfwCreateWindow, ICallable::INF_ARGS, "glfwCreateWindow")},
    {"glfwDestroyWindow", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwDestroyWindow, ICallable::INF_ARGS, "glfwDestroyWindow")},
    {"glfwMakeContextCurrent", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwMakeContextCurrent, ICallable::INF_ARGS, "glfwMakeContextCurrent")},
    {"glfwWindowShouldClose", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwWindowShouldClose, ICallable::INF_ARGS, "glfwWindowShouldClose")},
    {"glfwSetWindowShouldClose", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::INT}, wrapper_glfwSetWindowShouldClose, ICallable::INF_ARGS, "glfwSetWindowShouldClose")},
    {"glfwPollEvents", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwPollEvents, ICallable::INF_ARGS, "glfwPollEvents")},
    {"glfwSwapBuffers", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwSwapBuffers, ICallable::INF_ARGS, "glfwSwapBuffers")},
    {"glfwGetKey", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::INT}, wrapper_glfwGetKey, ICallable::INF_ARGS, "glfwGetKey")},
    {"glfwGetTime", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwGetTime, ICallable::INF_ARGS, "glfwGetTime")},
    {"glfwWindowHint", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT, ValueType::INT}, wrapper_glfwWindowHint, ICallable::INF_ARGS, "glfwWindowHint")},
    {"glfwSwapInterval", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT}, wrapper_glfwSwapInterval, ICallable::INF_ARGS, "glfwSwapInterval")},
};

 EXPORT NativeFunc GetCallable(const char* name) {
 return   *nativeCalls.at(name);
}
}
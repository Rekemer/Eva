#include "glfw_wrappers.h"
#include <glfw3.h>
#include "PluginData.h"
#include "Value.h"
#include "CallState.h"
#include <cassert>

namespace Eva {

#define EXPORT extern "C" __declspec(dllexport)


typedef void (*GLFWwindowsizefun)(GLFWwindow*, int, int);
typedef void (*GLFWwindowclosefun)(GLFWwindow*);

EXPORT int wrapper_glfwInit(CallState& st) {

// remove callable
	st.stack.pop_back();
   eint result = glfwInit();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwTerminate(CallState& st) {

// remove callable
	st.stack.pop_back();
    glfwTerminate();
// no return
    return 0;
}

EXPORT int wrapper_glfwCreateWindow(CallState& st) {
    auto arg4 = st.stack.back();
    st.stack.pop_back();    auto arg3 = st.stack.back();
    st.stack.pop_back();    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
   void* result = glfwCreateWindow(static_cast<int>(arg0.As<eint>()), static_cast<int>(arg1.As<eint>()), arg2.AsString().data(), reinterpret_cast<GLFWmonitor*>(arg3.As<eptr>()), reinterpret_cast<GLFWwindow*>(arg4.As<eptr>()));
 st.stack.push_back(reinterpret_cast<eptr>(result));
    return 1;
}

EXPORT int wrapper_glfwDestroyWindow(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
// no return
    return 0;
}

EXPORT int wrapper_glfwMakeContextCurrent(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
// no return
    return 0;
}

EXPORT int wrapper_glfwWindowShouldClose(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
   eint result = glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwSetWindowShouldClose(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), static_cast<int>(arg1.As<eint>()));
// no return
    return 0;
}

EXPORT int wrapper_glfwPollEvents(CallState& st) {

// remove callable
	st.stack.pop_back();
    glfwPollEvents();
// no return
    return 0;
}

EXPORT int wrapper_glfwSwapBuffers(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
// no return
    return 0;
}

EXPORT int wrapper_glfwGetKey(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
   eint result = glfwGetKey(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), static_cast<int>(arg1.As<eint>()));
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwGetTime(CallState& st) {

// remove callable
	st.stack.pop_back();
   efloat result = glfwGetTime();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwWindowHint(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwWindowHint(static_cast<int>(arg0.As<eint>()), static_cast<int>(arg1.As<eint>()));
// no return
    return 0;
}

EXPORT int wrapper_glfwSwapInterval(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glfwSwapInterval(static_cast<int>(arg0.As<eint>()));
// no return
    return 0;
}

CallState* callState = nullptr;

std::unordered_map<std::string_view,eCallable> windowCallbacks;

void bridging_GLFWwindowsizefun(GLFWwindow* arg0, int arg1, int arg2) {
    auto it = windowCallbacks.find("bridging_GLFWwindowsizefun");
    if (it != windowCallbacks.end()) {
        
        auto userCallback = it->second;
         callState->deferredCallbacks.push([=]() {
             CallUserCallback(userCallback, callState, std::vector<ValueContainer>{ValueContainer(reinterpret_cast<eptr>(arg0)), ValueContainer(static_cast<eint>(arg1)), ValueContainer(static_cast<eint>(arg2))});
        });
       
    }
}

void bridging_GLFWwindowclosefun(GLFWwindow* arg0) {
    auto it = windowCallbacks.find("bridging_GLFWwindowclosefun");
    if (it != windowCallbacks.end()) {
        
        auto userCallback = it->second;
         callState->deferredCallbacks.push([=]() {
             CallUserCallback(userCallback, callState, std::vector<ValueContainer>{ValueContainer(reinterpret_cast<eptr>(arg0))});
        });
       
    }
}

EXPORT int wrapper_glfwSetWindowSizeCallback(CallState& st) {
    // Extract user-defined callback (function in bytecode)
    auto arg2 = st.stack.back(); st.stack.pop_back();
    auto userCallback = arg2.AsCallable();
    

    // Extract window handle
    auto arg1 = st.stack.back(); st.stack.pop_back();
    auto window = reinterpret_cast<GLFWwindow*>(arg1.As<eptr>());
    

    callState = &st;
    // Register the bridging function in GLFW
    windowCallbacks["bridging_GLFWwindowsizefun"] = userCallback;
     st.stack.pop_back();
    glfwSetWindowSizeCallback(window, bridging_GLFWwindowsizefun);
    return 0; // No return values
}

EXPORT int wrapper_glfwSetWindowCloseCallback(CallState& st) {
    // Extract user-defined callback (function in bytecode)
    auto arg2 = st.stack.back(); st.stack.pop_back();
    auto userCallback = arg2.AsCallable();
    

    // Extract window handle
    auto arg1 = st.stack.back(); st.stack.pop_back();
    auto window = reinterpret_cast<GLFWwindow*>(arg1.As<eptr>());
    

    callState = &st;
    // Register the bridging function in GLFW
    windowCallbacks["bridging_GLFWwindowclosefun"] = userCallback;
     st.stack.pop_back();
    glfwSetWindowCloseCallback(window, bridging_GLFWwindowclosefun);
    return 0; // No return values
}

std::unordered_map<std::string, int(*)(CallState&)> functionTable = {
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

std::unordered_map<std::string, std::shared_ptr<NativeFunc>> nativeCalls = {
    {"glfwInit", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwInit, ICallable::INF_ARGS, "glfwInit", CallFlags::ExternalDLL)},
    {"glfwTerminate", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwTerminate, ICallable::INF_ARGS, "glfwTerminate", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwCreateWindow", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT, ValueType::INT, ValueType::STRING, ValueType::PTR, ValueType::PTR}, wrapper_glfwCreateWindow, ICallable::INF_ARGS, "glfwCreateWindow", CallFlags::ExternalDLL)},
    {"glfwDestroyWindow", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwDestroyWindow, ICallable::INF_ARGS, "glfwDestroyWindow", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwMakeContextCurrent", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwMakeContextCurrent, ICallable::INF_ARGS, "glfwMakeContextCurrent", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwWindowShouldClose", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwWindowShouldClose, ICallable::INF_ARGS, "glfwWindowShouldClose", CallFlags::ExternalDLL)},
    {"glfwSetWindowShouldClose", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::INT}, wrapper_glfwSetWindowShouldClose, ICallable::INF_ARGS, "glfwSetWindowShouldClose", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwPollEvents", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwPollEvents, ICallable::INF_ARGS, "glfwPollEvents", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwSwapBuffers", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR}, wrapper_glfwSwapBuffers, ICallable::INF_ARGS, "glfwSwapBuffers", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwGetKey", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::INT}, wrapper_glfwGetKey, ICallable::INF_ARGS, "glfwGetKey", CallFlags::ExternalDLL)},
    {"glfwGetTime", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glfwGetTime, ICallable::INF_ARGS, "glfwGetTime", CallFlags::ExternalDLL)},
    {"glfwWindowHint", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT, ValueType::INT}, wrapper_glfwWindowHint, ICallable::INF_ARGS, "glfwWindowHint", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwSwapInterval", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT}, wrapper_glfwSwapInterval, ICallable::INF_ARGS, "glfwSwapInterval", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glfwSetWindowSizeCallback", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::FUNCTION}, wrapper_glfwSetWindowSizeCallback, ICallable::INF_ARGS, "glfwSetWindowSizeCallback", CallFlags::ExternalDLL)},
    {"glfwSetWindowCloseCallback", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::PTR, ValueType::FUNCTION}, wrapper_glfwSetWindowCloseCallback, ICallable::INF_ARGS, "glfwSetWindowCloseCallback", CallFlags::ExternalDLL)},
};



 EXPORT std::shared_ptr<ICallable> GetCallable(const char* name) {
 return   nativeCalls.at(name);
}
std::unordered_map<std::string, eint> constants = {
    {"GLFW_KEY_ESCAPE", 256},
    {"GLFW_KEY_SPACE", 32},
    {"GLFW_KEY_S", 83},
    {"GLFW_KEY_W", 87},
    {"GLFW_PRESS", 1},
    {"GLFW_KEY_UP", 265},
    {"GLFW_KEY_DOWN", 264},
};
 EXPORT TypeTable* getTypeTable() {
    auto typeMap = new TypeTable{} ;
    (*typeMap)["glfwInit"] = std::vector<ValueType>{ValueType::INT};
    (*typeMap)["glfwTerminate"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glfwCreateWindow"] = std::vector<ValueType>{ValueType::PTR, ValueType::INT, ValueType::INT, ValueType::STRING, ValueType::PTR, ValueType::PTR};
    (*typeMap)["glfwDestroyWindow"] = std::vector<ValueType>{ValueType::NIL, ValueType::PTR};
    (*typeMap)["glfwMakeContextCurrent"] = std::vector<ValueType>{ValueType::NIL, ValueType::PTR};
    (*typeMap)["glfwWindowShouldClose"] = std::vector<ValueType>{ValueType::INT, ValueType::PTR};
    (*typeMap)["glfwSetWindowShouldClose"] = std::vector<ValueType>{ValueType::NIL, ValueType::PTR, ValueType::INT};
    (*typeMap)["glfwPollEvents"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glfwSwapBuffers"] = std::vector<ValueType>{ValueType::NIL, ValueType::PTR};
    (*typeMap)["glfwGetKey"] = std::vector<ValueType>{ValueType::INT, ValueType::PTR, ValueType::INT};
    (*typeMap)["glfwGetTime"] = std::vector<ValueType>{ValueType::FLOAT};
    (*typeMap)["glfwWindowHint"] = std::vector<ValueType>{ValueType::NIL, ValueType::INT, ValueType::INT};
    (*typeMap)["glfwSwapInterval"] = std::vector<ValueType>{ValueType::NIL, ValueType::INT};
    (*typeMap)["glfwSetWindowSizeCallback"] = std::vector<ValueType>{ValueType::FUNCTION, ValueType::PTR, ValueType::FUNCTION};
    (*typeMap)["glfwSetWindowCloseCallback"] = std::vector<ValueType>{ValueType::FUNCTION, ValueType::PTR, ValueType::FUNCTION};

    return typeMap;
}
 EXPORT ConstTable* getConstTable() {
   return &constants;
}} // namespace Eva

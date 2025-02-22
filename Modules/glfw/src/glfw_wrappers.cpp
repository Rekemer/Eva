#include "glfw_wrappers.h"
#include <glfw3.h>
#include "PluginData.h"
#include "Value.h"
#include "CallState.h"
#include <cassert>

namespace Eva {

#define EXPORT extern "C" __declspec(dllexport)

EXPORT int wrapper_glfwInit(CallState& st) {

   eint result = glfwInit();
	st.stack.pop_back();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwTerminate(CallState& st) {

    glfwTerminate();
	st.stack.pop_back();
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
   void* result = glfwCreateWindow(static_cast<int>(arg0.As<eint>()), static_cast<int>(arg1.As<eint>()), arg2.AsString().data(), reinterpret_cast<GLFWmonitor*>(arg3.As<eptr>()), reinterpret_cast<GLFWwindow*>(arg4.As<eptr>()));
	st.stack.pop_back();
 st.stack.push_back(reinterpret_cast<eptr>(result));
    return 1;
}

EXPORT int wrapper_glfwDestroyWindow(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwMakeContextCurrent(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwMakeContextCurrent(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwWindowShouldClose(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
   eint result = glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
	st.stack.pop_back();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwSetWindowShouldClose(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), static_cast<int>(arg1.As<eint>()));
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwPollEvents(CallState& st) {

    glfwPollEvents();
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwSwapBuffers(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()));
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwGetKey(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
   eint result = glfwGetKey(reinterpret_cast<GLFWwindow*>(arg0.As<eptr>()), static_cast<int>(arg1.As<eint>()));
	st.stack.pop_back();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwGetTime(CallState& st) {

   efloat result = glfwGetTime();
	st.stack.pop_back();
   st.stack.push_back(result);
    return 1;
}

EXPORT int wrapper_glfwWindowHint(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwWindowHint(static_cast<int>(arg0.As<eint>()), static_cast<int>(arg1.As<eint>()));
	st.stack.pop_back();
// no return
    return 0;
}

EXPORT int wrapper_glfwSwapInterval(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
    glfwSwapInterval(static_cast<int>(arg0.As<eint>()));
	st.stack.pop_back();
// no return
    return 0;
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
};



 EXPORT std::shared_ptr<ICallable> GetCallable(const char* name) {
 return   nativeCalls.at(name);
}
std::unordered_map<std::string, eint> constants = {
    {"GLFW_KEY_ESCAPE", 256},
    {"GLFW_PRESS", 1},
};
 EXPORT TypeTable* getTypeTable() {
    auto typeMap = new TypeTable{} ;
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
}
 EXPORT ConstTable* getConstTable() {
   return &constants;
}} // namespace Eva

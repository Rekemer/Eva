#include "opengl-legacy_wrappers.h"
#include <windows.h>
#include <GL/gl.h>
#include "PluginData.h"
#include "Value.h"
#include "CallState.h"
#include <cassert>

namespace Eva {

#define EXPORT extern "C" __declspec(dllexport)



EXPORT int wrapper_glClearColor(CallState& st) {
    auto arg3 = st.stack.back();
    st.stack.pop_back();    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glClearColor(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()), static_cast<GLfloat>(arg2.As<efloat>()), static_cast<GLfloat>(arg3.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glClear(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glClear(static_cast<GLbitfield>(arg0.As<eint>()));
// no return
    return 0;
}

EXPORT int wrapper_glBegin(CallState& st) {
    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glBegin(static_cast<GLenum>(arg0.As<eint>()));
// no return
    return 0;
}

EXPORT int wrapper_glVertex2f(CallState& st) {
    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glVertex2f(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glVertex3f(CallState& st) {
    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glVertex3f(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()), static_cast<GLfloat>(arg2.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glColor3f(CallState& st) {
    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glColor3f(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()), static_cast<GLfloat>(arg2.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glEnd(CallState& st) {

// remove callable
	st.stack.pop_back();
    glEnd();
// no return
    return 0;
}

EXPORT int wrapper_glLoadIdentity(CallState& st) {

// remove callable
	st.stack.pop_back();
    glLoadIdentity();
// no return
    return 0;
}

EXPORT int wrapper_glPushMatrix(CallState& st) {

// remove callable
	st.stack.pop_back();
    glPushMatrix();
// no return
    return 0;
}

EXPORT int wrapper_glPopMatrix(CallState& st) {

// remove callable
	st.stack.pop_back();
    glPopMatrix();
// no return
    return 0;
}

EXPORT int wrapper_glTranslatef(CallState& st) {
    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glTranslatef(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()), static_cast<GLfloat>(arg2.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glRotatef(CallState& st) {
    auto arg3 = st.stack.back();
    st.stack.pop_back();    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glRotatef(static_cast<GLfloat>(arg0.As<efloat>()), static_cast<GLfloat>(arg1.As<efloat>()), static_cast<GLfloat>(arg2.As<efloat>()), static_cast<GLfloat>(arg3.As<efloat>()));
// no return
    return 0;
}

EXPORT int wrapper_glViewport(CallState& st) {
    auto arg3 = st.stack.back();
    st.stack.pop_back();    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glViewport(static_cast<GLint>(arg0.As<eint>()), static_cast<GLint>(arg1.As<eint>()), static_cast<GLint>(arg2.As<eint>()), static_cast<GLint>(arg3.As<eint>()));
// no return
    return 0;
}

EXPORT int wrapper_glOrtho(CallState& st) {
    auto arg5 = st.stack.back();
    st.stack.pop_back();    auto arg4 = st.stack.back();
    st.stack.pop_back();    auto arg3 = st.stack.back();
    st.stack.pop_back();    auto arg2 = st.stack.back();
    st.stack.pop_back();    auto arg1 = st.stack.back();
    st.stack.pop_back();    auto arg0 = st.stack.back();
    st.stack.pop_back();
// remove callable
	st.stack.pop_back();
    glOrtho(static_cast<GLdouble>(arg0.As<efloat>()), static_cast<GLdouble>(arg1.As<efloat>()), static_cast<GLdouble>(arg2.As<efloat>()), static_cast<GLdouble>(arg3.As<efloat>()), static_cast<GLdouble>(arg4.As<efloat>()), static_cast<GLdouble>(arg5.As<efloat>()));
// no return
    return 0;
}

CallState* callState = nullptr;

std::unordered_map<std::string, int(*)(CallState&)> functionTable = {
    {"glClearColor", &wrapper_glClearColor},
    {"glClear", &wrapper_glClear},
    {"glBegin", &wrapper_glBegin},
    {"glVertex2f", &wrapper_glVertex2f},
    {"glVertex3f", &wrapper_glVertex3f},
    {"glColor3f", &wrapper_glColor3f},
    {"glEnd", &wrapper_glEnd},
    {"glLoadIdentity", &wrapper_glLoadIdentity},
    {"glPushMatrix", &wrapper_glPushMatrix},
    {"glPopMatrix", &wrapper_glPopMatrix},
    {"glTranslatef", &wrapper_glTranslatef},
    {"glRotatef", &wrapper_glRotatef},
    {"glViewport", &wrapper_glViewport},
    {"glOrtho", &wrapper_glOrtho},
};

std::unordered_map<std::string, std::shared_ptr<NativeFunc>> nativeCalls = {
    {"glClearColor", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glClearColor, ICallable::INF_ARGS, "glClearColor", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glClear", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT}, wrapper_glClear, ICallable::INF_ARGS, "glClear", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glBegin", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT}, wrapper_glBegin, ICallable::INF_ARGS, "glBegin", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glVertex2f", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT}, wrapper_glVertex2f, ICallable::INF_ARGS, "glVertex2f", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glVertex3f", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glVertex3f, ICallable::INF_ARGS, "glVertex3f", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glColor3f", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glColor3f, ICallable::INF_ARGS, "glColor3f", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glEnd", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glEnd, ICallable::INF_ARGS, "glEnd", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glLoadIdentity", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glLoadIdentity, ICallable::INF_ARGS, "glLoadIdentity", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glPushMatrix", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glPushMatrix, ICallable::INF_ARGS, "glPushMatrix", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glPopMatrix", std::make_shared<NativeFunc>(std::vector<ValueType>{}, wrapper_glPopMatrix, ICallable::INF_ARGS, "glPopMatrix", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glTranslatef", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glTranslatef, ICallable::INF_ARGS, "glTranslatef", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glRotatef", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glRotatef, ICallable::INF_ARGS, "glRotatef", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glViewport", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::INT, ValueType::INT, ValueType::INT, ValueType::INT}, wrapper_glViewport, ICallable::INF_ARGS, "glViewport", CallFlags::ExternalDLL | CallFlags::VoidCall)},
    {"glOrtho", std::make_shared<NativeFunc>(std::vector<ValueType>{ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT}, wrapper_glOrtho, ICallable::INF_ARGS, "glOrtho", CallFlags::ExternalDLL | CallFlags::VoidCall)},
};



 EXPORT std::shared_ptr<ICallable> GetCallable(const char* name) {
 return   nativeCalls.at(name);
}
std::unordered_map<std::string, eint> constants = {
    {"GL_COLOR_BUFFER_BIT", 16384},
    {"GL_DEPTH_BUFFER_BIT", 256},
    {"GL_TRIANGLES", 4},
    {"GL_LINES", 1},
    {"GL_POINTS", 0},
    {"GL_QUADS", 7},
};
 EXPORT TypeTable* getTypeTable() {
    auto typeMap = new TypeTable{} ;
    (*typeMap)["glClearColor"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glClear"] = std::vector<ValueType>{ValueType::NIL, ValueType::INT};
    (*typeMap)["glBegin"] = std::vector<ValueType>{ValueType::NIL, ValueType::INT};
    (*typeMap)["glVertex2f"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glVertex3f"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glColor3f"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glEnd"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glLoadIdentity"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glPushMatrix"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glPopMatrix"] = std::vector<ValueType>{ValueType::NIL};
    (*typeMap)["glTranslatef"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glRotatef"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};
    (*typeMap)["glViewport"] = std::vector<ValueType>{ValueType::NIL, ValueType::INT, ValueType::INT, ValueType::INT, ValueType::INT};
    (*typeMap)["glOrtho"] = std::vector<ValueType>{ValueType::NIL, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT, ValueType::FLOAT};

    return typeMap;
}
 EXPORT ConstTable* getConstTable() {
   return &constants;
}} // namespace Eva

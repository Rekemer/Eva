#pragma once
namespace Eva {
#ifdef __cplusplus
extern "C" {
#endif

struct CallState;

// A single function pointer type for all GLFW wrappers
typedef int (*GLFW_WrapperFn)(CallState& st);

typedef struct {
    const char* name;       // e.g. "glfwInit"
    GLFW_WrapperFn wrapper; // e.g. pointer to wrapper_glfwInit
} GLFWFuncEntry;

extern GLFWFuncEntry g_glfwFunctionTable[];

// For demonstration, we declare a "parse" function or macros your real code might use
// But real code might #include <GLFW/glfw3.h> or do dynamic loading.

#ifdef __cplusplus
}
#endif

}
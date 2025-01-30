#!/usr/bin/env python3
import yaml
from state import * 
# We'll define a skeleton for the "MyState" approach
# so each generated wrapper is "int wrapper_xyz(MyState* st)".

HEADER_TEMPLATE = r'''#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MyState MyState;

// A single function pointer type for all GLFW wrappers
typedef int (*GLFW_WrapperFn)(MyState* st);

typedef struct {
    const char* name;       // e.g. "glfwInit"
    GLFW_WrapperFn wrapper; // e.g. pointer to wrapper_glfwInit
} GLFWFuncEntry;

extern GLFWFuncEntry g_glfwFunctionTable[];
extern int g_glfwFunctionCount;

// For demonstration, we declare a "parse" function or macros your real code might use
// But real code might #include <GLFW/glfw3.h> or do dynamic loading.

#ifdef __cplusplus
}
#endif
'''


def get_c_value(cType):
    isPointer = '*' in cType
    isChar = 'char' in cType
    if isPointer and not isChar:
        return f"As<eptr>()"
    if cType == "double" or cType == "float":
        return "As<efloat>"
    elif cType == "bool":
        return "As<ebool>()"
    elif cType == "int":
        return "As<eint>()"
    if isChar:
        return "AsString()";
    return "LOX"

def check_cast(cType,arg):
    isPointer = '*' in cType
    if isPointer:
        return f"reinterpeter_cast<{cType}>({arg})"
    else:
        return arg
def generate_prototypes_and_wrappers(data):
    """
    data is the parsed YAML with keys:
    data["functions"]: list of { name: str, return: str, args: [{type: str},...] }
    We'll generate:

      int wrapper_<funcName>(MyState* st) {
         // parse arguments from st
         // call real <funcName>(...)
         // push results
         return number_of_results;
      }
    """

    function_table_entries = []
    wrapper_defs = []

    for func in data["functions"]:
        fn_name = func["name"]              # e.g. glfwInit
        ret_type = func["return"]           # e.g. int, void, pointer, etc.
        arg_list = func["args"]             # e.g. [ {type: int}, {type: int}, ... ]
        is_ret_ptr = "*" in ret_type
        # We'll define "int wrapper_glfwInit(MyState* st)"
        wrapper_name = f"wrapper_{fn_name}"

        # We'll create a skeleton that prints placeholders
        # In a real system, you'd parse st for arguments, call <fn_name>, etc.
        # We'll just do some pseudo-code for demonstration.

        # Pseudo parse code
        parse_lines = []
        size = len(arg_list) - 1
        for i, arg in enumerate(arg_list):
            argCType = arg["type"]    # e.g. "int", "pointer", ...
            #parse_lines.append(f"    // {argCType} arg{i} = ??? // parse from st")
            parse_lines.append(f"      auto arg{size - i} = st->stack->pop_back(); // parse from st")
        
        # Actual call
        # If return is "void", we won't store result
        # If return is "int" or "pointer", we do something
        call_args = ", ".join([f"{check_cast(arg['type'],'arg{i}')}.{get_c_value(arg['type'])}" for i, arg in enumerate(arg_list)])
        if ret_type == "void":
            call_line = f"    {fn_name}({call_args});"
            ret_push = "// no return, so no results to push"
            result_count = "0"
        elif ret_type == "int" or ret_type == "float" or ret_type == "double" or ret_type == "bool" :
            call_line = f"    auto result = {fn_name}({call_args});"
            ret_push = "st->stack->push_back(ValueContainer(result));"
            result_count = "1"
        elif is_ret_ptr:
            call_line = f"    void* result = {fn_name}({call_args});"
            ret_push = "st->stack->push_back(ValueContainer(reinterpret_cast<eptr>(result)));"
            result_count = "1"
        else:
            # fallback
            call_line = f"    /* unhandled return type {ret_type} */"
            ret_push = "assert(false);"
            result_count = "0"
        string6 = '\n'
        wrapper_def = f"""int {wrapper_name}(MyState* st) {{
    
{string6.join(parse_lines)}
{call_line}
    {ret_push}
    return {result_count}; // number of values we 'return'
}}"""

        wrapper_defs.append(wrapper_def)

        # Add to function table
        function_table_entries.append(f'    {{"{fn_name}", &{wrapper_name}}},')

    return wrapper_defs, function_table_entries


def main():
    with open("glfw_api.yaml", "r") as f:
        data = yaml.safe_load(f)

    # Generate the code
    wrapper_defs, function_table_entries = generate_prototypes_and_wrappers(data)

    # Build the .h output
    with open("glfw_wrappers.h", "w") as hf:
        hf.write(HEADER_TEMPLATE)

    # Build the .cpp output
    with open("glfw_wrappers.cpp", "w") as sf:
        sf.write(SOURCE_HEADER)
        sf.write("\n")

        for wd in wrapper_defs:
            sf.write(wd)
            sf.write("\n\n")

        # define function table
        sf.write("GLFWFuncEntry g_glfwFunctionTable[] = {\n")
        for entry in function_table_entries:
            sf.write(entry + "\n")
        sf.write("};\n\n")
        sf.write("int g_glfwFunctionCount = sizeof(g_glfwFunctionTable)/sizeof(GLFWFuncEntry);\n")

        sf.write("\n")


if __name__ == "__main__":
    main()

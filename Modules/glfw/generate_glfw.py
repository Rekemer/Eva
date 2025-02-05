#!/usr/bin/env python3
import yaml
from state import * 
# We'll define a skeleton for the "MyState" approach
# so each generated wrapper is "int wrapper_xyz(MyState* st)".

HEADER_TEMPLATE = r'''#pragma once
#include "ICallable.h"
#include "Function.h"
namespace Eva {
extern "C" {
struct CallState;


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
        return "AsString().data()";
    return "LOX"

def check_cast(cType,arg):
    isPointer = '*' in cType
    callArg = f"{arg}.{get_c_value(cType)}"
    if isPointer:
        return f"reinterpret_cast<{cType}>({callArg})"
    else:
        return callArg
def generate_prototypes_and_wrappers(data):
    """
    data is the parsed YAML with keys:
    data["functions"]: list of { name: str, return: str, args: [{type: str},...] }
    We'll generate:

      int wrapper_<funcName>(CallState* st) {
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
            parse_lines.append(f"      auto arg{size - i} = st.stack.back();\n\tst.stack.pop_back();")
        
        # Actual call
        # If return is "void", we won't store result
        # If return is "int" or "pointer", we do something
        call_args = ", ".join([f"{check_cast(arg['type'],f'arg{i}')}" for i, arg in enumerate(arg_list)])
        if ret_type == "void":
            call_line = f"    {fn_name}({call_args});"
            ret_push = "// no return, so no results to push"
            result_count = "0"
        elif ret_type == "int" or ret_type == "float" or ret_type == "double" or ret_type == "bool" :
            call_line = f"    auto result = {fn_name}({call_args});"
            ret_push = "st.stack.push_back(ValueContainer(result));"
            result_count = "1"
        elif is_ret_ptr:
            call_line = f"    void* result = {fn_name}({call_args});"
            ret_push = "st.stack.push_back(ValueContainer(reinterpret_cast<eptr>(result)));"
            result_count = "1"
        else:
            # fallback
            call_line = f"    /* unhandled return type {ret_type} */"
            ret_push = "assert(false);"
            result_count = "0"
        string6 = '\n'
        wrapper_def = f""" EXPORT int {wrapper_name}(CallState& st) {{
    
{string6.join(parse_lines)}
{call_line}
    {ret_push}
    return {result_count}; // number of values we 'return'
}}"""

        wrapper_defs.append(wrapper_def)

        # Add to function table
        function_table_entries.append(f'    {{"{fn_name}", &{wrapper_name}}},')

    return wrapper_defs, function_table_entries

# Suppose we have a dictionary describing how each C type maps to ValueType
# In real usage, you could parse a YAML or JSON file. Here we just define inline.
C_TYPE_MAP = {
    "bool":       "ValueType::BOOL",
    "int":        "ValueType::INT",
    "float":      "ValueType::FLOAT",
    "double":     "ValueType::FLOAT",   # or a separate ValueType::DOUBLE if you prefer
    "GLFWwindow*":  "ValueType::PTR",
    "GLFWmonitor*": "ValueType::PTR",
    "const char*":  "ValueType::STRING",
    "void" : "ValueType::NIL"
}

def generate_calltable(data):
    
    lines = []
    lines.append("std::unordered_map<std::string, std::shared_ptr<NativeFunc>> nativeCalls = {")
    
    for func in data["functions"]:
        name = func["name"]
        # Build a vector of ValueType for each argument.
        arg_types = []
        for arg in func["args"]:
            ctype = arg["type"]
            if ctype in C_TYPE_MAP:
                arg_types.append(C_TYPE_MAP[ctype])
            else:
                # Fallback if type is unknown.
                arg_types.append("ValueType::NIL")
        if arg_types:
            vec_literal = "std::vector<ValueType>{" + ", ".join(arg_types) + "}"
            arg_kind = "ICallable::INF_ARGS"
        else:
            vec_literal = "std::vector<ValueType>{}"
            arg_kind = "ICallable::INF_ARGS"
        
        # The wrapper function is assumed to be named "wrapper_<name>"
        wrapper_name = f"wrapper_{name}"
        # Generate a table entry line.
        line = f'    {{"{name}", std::make_shared<NativeFunc>({vec_literal}, {wrapper_name}, {arg_kind}, "{name}")}},'
        lines.append(line)
    
    lines.append("};")
    return lines
def generate_callgetter(data):
    lines = []
    lines.append("\n")
    lines.append(" EXPORT NativeFunc GetCallable(const char* name) {")
    lines.append(" return   *nativeCalls.at(name);")
    lines.append("}")
    return lines
def generate_metatable(data):
    
    lines = []
    lines.append(" EXPORT std::unordered_map<std::string, ValueType>* initModule() {")
    lines.append("    std::unordered_map<std::string, ValueType>* typeMap = new std::unordered_map<std::string, ValueType>{} ;")
    for func in data["functions"]:
        ret_type = func["return"]
        line = f'    (*typeMap)["{func["name"]}"] = {C_TYPE_MAP[ret_type]};'
        lines.append(line)
    # For each entry in C_TYPE_MAP, generate code like:
    #    typeMap["int"] = ValueType::INT;
        #for ctype, valtype in C_TYPE_MAP.items():
            
    lines.append("")
    lines.append("    return typeMap;")
    lines.append("}")
    return lines
    

def main():
    with open("glfw_api.yaml", "r") as f:
        data = yaml.safe_load(f)

    # Generate the code
    wrapper_defs, function_table_entries = generate_prototypes_and_wrappers(data)
    # Build the .h output
    lines = generate_metatable(data)
    with open("./src/glfw_wrappers.h", "w+") as hf:
        hf.write(HEADER_TEMPLATE)
        #hf.write("\n".join(lines))
        # for namespace
        hf.write('\n}')
        # for extern 
        hf.write('\n}')

    lines_calls = generate_calltable(data)
    lines_getter = generate_callgetter(data)
    # Build the .cpp output
    with open("./src/glfw_wrappers.cpp", "w+") as sf:
        sf.write(SOURCE_HEADER)
        sf.write("\n")

        for wd in wrapper_defs:
            sf.write(wd)
            sf.write("\n\n")

        # define function table
        sf.write("\n".join(lines))
        sf.write("\n".join(lines_calls))
        sf.write("\n".join(lines_getter))
        sf.write("\n}")
    


if __name__ == "__main__":
    main()

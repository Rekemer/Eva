#!/usr/bin/env python3
import yaml
import sys
import os

# Maps C types to your language's ValueType or handling
C_TYPE_MAP = {
    "bool": "ValueType::BOOL",
    "int": "ValueType::INT",
    "float": "ValueType::FLOAT",
    "double": "ValueType::FLOAT",  # or separate "DOUBLE"
    "void": "ValueType::NIL",
    # Common pointers for e.g. GLFW or GL
    "GLFWwindow*": "ValueType::PTR",
    "GLFWmonitor*": "ValueType::PTR",
    "const char*": "ValueType::STRING",
    "GLenum": "ValueType::INT",
    "GLfloat": "ValueType::FLOAT",
    "GLint": "ValueType::INT",
}


C_EVA_MAP = {
    "bool":       "ebool",
    "int":        "eint",
    "GLint":        "eint",
    "GLenum":        "eint",
    "float":      "efloat",
    "GLfloat":      "efloat",
    "double":     "efloat",   
    "GLFWwindow*":  "eptr",
    "GLFWmonitor*": "eptr",
    "const char*":  "estring",
    "GLbitfield":"eint",
}
def put_mapped_type(cType):
    return C_EVA_MAP[cType]

def castArgEva(cType, argName):
    """
    Return code that casts from 'argName.As<xyz>()' to the real cType.
    E.g. if cType = "int", -> static_cast<int>(argName.As<eint>()).
    If pointer, reinterpret.
    """
    isChar = 'char' in cType
    if "*" in cType and not isChar:
        return f"reinterpret_cast<{put_mapped_type(cType)}>({argName})"
    elif isChar:
        return f"{argName}"
    else:
        return f"static_cast<{put_mapped_type(cType)}>({argName})"
    #elif cType in ["float","double","GLfloat"]:
    #    return f"static_cast<{cType}>({put_mapped_type(cType)}.As<efloat>())"
    #elif cType in ["int","GLint","GLenum","bool"]:
    #    return f"static_cast<{cType}>({argName}.As<eint>())"
    return argName

def castArg(cType, argName):
    """
    Return code that casts from 'argName.As<xyz>()' to the real cType.
    E.g. if cType = "int", -> static_cast<int>(argName.As<eint>()).
    If pointer, reinterpret.
    """
    isChar = 'char' in cType
    if "*" in cType and not isChar:
        return f"reinterpret_cast<{cType}>({argName}.As<eptr>())"
    elif isChar:
        return f"{argName}.AsString().data()"
    else:
        return f"static_cast<{cType}>({argName}.As<{put_mapped_type(cType)}>())"
    #elif cType in ["float","double","GLfloat"]:
    #    return f"static_cast<{cType}>({put_mapped_type(cType)}.As<efloat>())"
    #elif cType in ["int","GLint","GLenum","bool"]:
    #    return f"static_cast<{cType}>({argName}.As<eint>())"
    return argName

def generate_callgetter():
    lines =[]
    lines.append("\n")
    lines.append(" EXPORT std::shared_ptr<ICallable> GetCallable(const char* name) {")
    lines.append(" return   nativeCalls.at(name);")
    lines.append("}\n")
    return lines
def generate_metatable(data):
    
    lines = []
    lines.append(" EXPORT TypeTable* getTypeTable() {")
    lines.append("    auto typeMap = new TypeTable{} ;")
    for func in data["functions"]:
        ret_type = func["return"]
        line = f'    (*typeMap)["{func["name"]}"] = {C_TYPE_MAP[ret_type]};'
        lines.append(line)

            
    lines.append("")
    lines.append("    return typeMap;")
    lines.append("}")


    lines.append(" EXPORT ConstTable* getConstTable() {")
    lines.append("   return &constants;")
    lines.append("}")

    return lines


def generate_callback_wrappers(data):
    """
    Generate C++ wrapper functions for GLFW callbacks
    """
    wrapper_functions = []
    bridging_functions = []
    bridging_functions.append("CallState* callState = nullptr;")
    for table in data["tables"]:
        name = table["name"]
        key = table["key"]
        value = table["value"]
        bridging_functions.append(f"std::unordered_map<{key},{value}> {name};")
    
    for func in data["functions"]:
        fn_name = func["name"]  # Function name, e.g., glfwSetWindowSizeCallback
        ret_type = func["return"]
        args = func["args"]

        # Detect if any argument is a function pointer
        callback_arg = next((arg for arg in args if "fun" in arg["type"]), None)

        if callback_arg:
            callback_type = callback_arg["type"]  # Extract function pointer type
            callback_name = callback_type  # Same name as function pointer type
            
            # Generate Wrapper Function
            wrapper_fn = f"""EXPORT int wrapper_{fn_name}(CallState& st) {{
    // Extract user-defined callback (function in bytecode)
    auto arg2 = st.stack.back(); st.stack.pop_back();
    auto userCallback = arg2.AsCallable();
    

    // Extract window handle
    auto arg1 = st.stack.back(); st.stack.pop_back();
    auto window = reinterpret_cast<GLFWwindow*>(arg1.As<eptr>());
    
    // Store callback in our mapping
    windowCallbacks[window] = userCallback;

    callState = &st;
    // Register the bridging function in GLFW
    {fn_name}(window, bridging_{callback_name});
    return 0; // No return values
}}"""
            wrapper_functions.append(wrapper_fn)
            
            # Generate Bridging Function
            callback_def = next(c for c in data["callbacks"] if c["name"] == callback_name)
            callback_args = callback_def["args"]

            # Generate parameter list for bridging function
            bridging_args = ", ".join(f"{arg['type']} arg{i}" for i, arg in enumerate(callback_args))
            #bridging_call_args = ", ".join(f"ValueContainer({castArg(arg['type'],f'arg{i}')} )" for i, arg in enumerate(callback_args))
            bridging_call_args = ", ".join(f"ValueContainer({castArgEva(arg['type'], f'arg{i}')})" for i, arg in enumerate(callback_args))


            bridging_fn = f"""void bridging_{callback_name}({bridging_args}) {{
    auto it = windowCallbacks.find(arg0);
    if (it != windowCallbacks.end()) {{
        
        auto userCallback = it->second;
        CallUserCallback(userCallback, callState->GetStartIndex(), std::vector<ValueContainer>{{{bridging_call_args}}});
    }}
}}"""
            bridging_functions.append(bridging_fn)
    
    return wrapper_functions, bridging_functions




def generateWrappers(data):
    """
    data["functions"] -> list of function definitions
    Output:
      - wrapper functions
      - function pointer table entries
    """
    wrapper_defs = []
    table_entries = []


      # Defines the GLFW window resize callback function type
#   - name: GLFWwindowsizefun
#     return: void
#     args:
#       - type: "GLFWwindow*"  # Window handle
#       - type: int            # New width
#       - type: int            # New height
# typedef void (* GLFWwindowsizefun)(GLFWwindow* window, int width, int height);
    callbacks = data.get("callbacks", [])
    callback_lines = []
    for callback in callbacks:
        callback_lines .append("\n")
        fn = callback["name"]
        ret = callback["return"]
        a = callback.get("args", [])
        args_line = ", ".join(arg["type"] for arg in a) if a else "void"
        callback_lines.append(f"typedef {ret} (*{fn})({args_line});")
        C_EVA_MAP[fn] = "eCallable" 
        C_TYPE_MAP[fn] = "ValueType::FUNCTION" 
    #print(C_EVA_MAP)
    wrapper_defs.append("".join(callback_lines))
    functions = data.get("functions", [])
    for func in functions:
        fn = func["name"]
        ret = func["return"]

        is_callback = func.get("is_callback")  # Returns None if key is missing
        if is_callback == "true":
            continue

        is_ret_ptr = "*" in ret
        args = func.get("args", [])
        wrapperName = f"wrapper_{fn}"

        # For each arg, we pop from st.stack
        parse_lines = []
        size = len(args)-1
        for i, arg in enumerate(args):
            parse_lines.append(f"    auto arg{size-i} = st.stack.back();\n    st.stack.pop_back();")

        # Build call argument
        call_args = []
        for i, argDef in enumerate(args):
            ctype = argDef["type"]
            call_args.append(castArg(ctype, f"arg{i}"))
        callArgStr = ", ".join(call_args)

        # If ret = void, no push
        if ret == "void":
            call_line = f"    {fn}({callArgStr});"
            ret_push = "// no return"
            ret_count = "0"
        elif is_ret_ptr:
            call_line = f"   void* result = {fn}({callArgStr});"
            ret_push = " st.stack.push_back(reinterpret_cast<eptr>(result));"
            ret_count = "1"
        else:
            call_line = f"   {put_mapped_type(ret)} result = {fn}({callArgStr});"
            ret_push =  "   st.stack.push_back(result);"
            ret_count = "1"

        wdef = f"""EXPORT int {wrapperName}(CallState& st) {{
{''.join(parse_lines)}
// remove callable
\tst.stack.pop_back();
{call_line}
{ret_push}
    return {ret_count};
}}"""
        wrapper_defs.append(wdef)
        table_entries.append(f'    {{"{fn}", &{wrapperName}}},')

    return wrapper_defs, table_entries

def generateConstants(data):
    """
    data["constants"] -> create a map of name->value
    """
    lines = []
    lines.append("std::unordered_map<std::string, eint> constants = {")
    for c in data.get("constants", []):
        nm = c["name"]
        val = c["value"]
        lines.append(f'    {{"{nm}", {val}}},')
    lines.append("};")
    return lines

def generateCallTable(data):
    """
    Create the calltable of name->NativeFunc if you want 
    or you can skip if your code doesn't need it
    """
    lines = []
    lines.append("std::unordered_map<std::string, std::shared_ptr<NativeFunc>> nativeCalls = {")
    for func in data.get("functions", []):
        fnName = func["name"]
        args = func.get("args", [])
        # build vector of ValueType for the param 
        mapped = []
        for argDef in args:
            ctype = argDef["type"]
            mapped.append(C_TYPE_MAP.get(ctype,"ValueType::NIL"))
        if mapped:
            vec_literal = "std::vector<ValueType>{" + ", ".join(mapped) + "}"
            #arg_kind = "ICallable::FIXED_ARGS"
            arg_kind = "ICallable::INF_ARGS"
        else:
            vec_literal = "std::vector<ValueType>{}"
            arg_kind = "ICallable::INF_ARGS"
        wrapper = f"wrapper_{fnName}"
        is_void = func["return"] == "void"
        lines.append(f'    {{"{fnName}", std::make_shared<NativeFunc>({vec_literal}, {wrapper}, {arg_kind}, "{fnName}", {"CallFlags::ExternalDLL | CallFlags::VoidCall" if is_void else "CallFlags::ExternalDLL"})}},')
    lines.append("};")
    return lines

def main():
    if len(sys.argv) < 2:
        print("Usage: python generate_module.py <yaml_file>")
        return
    filepath = sys.argv[1]
    with open(filepath,"r") as f:
        data = yaml.safe_load(f)

    # read module name
    moduleName = data.get("moduleName","myModule")
    # read optional includes
    includes = data.get("includes", [])  # e.g. ["<GLFW/glfw3.h>","<cstdio>"]

    # generate code
    wrapper_defs, table_entries = generateWrappers(data)
    call_getter = generate_callgetter()
    const_table = generateConstants(data)
    call_table = generateCallTable(data)
    meta_table = generate_metatable(data)
    callback_wrappers, bridging_functions = generate_callback_wrappers(data)
    # produce filenames
    hFile = f"./{moduleName}/src/{moduleName}_wrappers.h"
    cFile = f"./{moduleName}/src/{moduleName}_wrappers.cpp"

    # Write the header
    with open(hFile,"w") as hf:
        hf.write(f"#pragma once\n #include <vector>\n #include \"Native.h\"\n#include \"ICallable.h\"\n#include \"Function.h\"\n#include <unordered_map>\n#include <string>\nnamespace Eva {{\nextern \"C\" {{\nstruct CallState;\n}}\n}}\n")

    # write the source
    with open(cFile,"w") as sf:
        # includes
        sf.write(f"#include \"{moduleName}_wrappers.h\"\n")
        for inc in includes:
            sf.write(f"#include {inc}\n")
        sf.write("#include \"PluginData.h\"\n#include \"Value.h\"\n#include \"CallState.h\"\n#include <cassert>\n\n")
        sf.write("namespace Eva {\n\n#define EXPORT extern \"C\" __declspec(dllexport)\n\n")

        # wrapper defs
        for wd in wrapper_defs:
            sf.write(wd+"\n\n")
        # Write bridging functions
        for bridge in bridging_functions:
            sf.write(bridge)
            sf.write("\n\n")

         # Write callback wrappers
        for cb_wrapper in callback_wrappers:
            sf.write(cb_wrapper)
            sf.write("\n\n")


        # function pointer table
        sf.write("std::unordered_map<std::string, int(*)(CallState&)> functionTable = {\n")
        for t in table_entries:
            sf.write(t + "\n")
        sf.write("};\n\n")
        # optional calltable with NativeFunc
        sf.write("\n".join(call_table))
        sf.write("\n\n")
        sf.write("\n".join(call_getter))

        # constants
        sf.write("\n".join(const_table))
        sf.write("\n")
        sf.write("\n".join(meta_table))
        sf.write("} // namespace Eva\n")

    print(f"Generated {hFile} and {cFile} for module: {moduleName}")


if __name__=="__main__":
    main()

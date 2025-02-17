# Eva programming language

The experimental basic toy language is inspired by Clox from *Crafting Interpreters*, however, Eva has an AST representation of the program and types.  
The main purpose of the language is to learn more about how to build them.

### Features
- **Constant Folding**: If possible, calculate constant values at compile time
- **Dead Variable Elmination**: Remove unused variables
- **Constant Propagation**: If variable is deduced to be constant, replace all variable usage as constant
- **SSA (Static Single Assignment)**: Represent variables with a single assignment each, using phi-nodes to handle merges at control flow joins
- **Control Flow Graph (CFG)**: Represent program execution paths as a graph
- **C FFI**: Call C functions from GLFW and legacy OpenGL by defining modular bindings that are dynamically loaded into the virtual machine.
### C FFI (GLFW and legacy OpenGL)
![](./demos/render_triangle/render_triangle.gif)

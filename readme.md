# Eva programming language

The experimental basic toy language is inspired by Clox from *Crafting Interpreters*, however, Eva has an AST representation of the program and types.  
The main purpose of the language is to learn more about how to build them.

### Features
- **Constant Folding**: If possible, calculate constant values at compile time
- **Constant Propagation**: If variable is deduced to be constant, replace all variable usage as constant
- **SSA (Static Single Assignment)**: Represent variables with a single assignment each, using phi-nodes to handle merges at control flow joins
- **Control Flow Graph (CFG)**: Represent program execution paths as a graph

### Example

```cpp
g := 2;

fun print(a: float, b: int) : int {
    Print g;
    c: int = b + a;
    return c;
}

fun a(): int {
    return 2;
}

fun main(): int {
    Print g;
    a := 2.0;
    print(1, a());
    return 0;
}
```
Example of other code can be found in tests

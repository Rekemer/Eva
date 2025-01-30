SOURCE_HEADER = r'''#include "glfw_wrappers.h"
#include  <stdio.h> // for demo printing
#include  <GLFW/glfw3.h> // or your dynamic load approach
#include  <vector>
#include  "Value.h"

// We'll define a dummy MyState to show usage:
struct MyState {
    // In real usage, you'd store a stack of values, or arguments, etc.
    // This is just a placeholder.
    std::vector<ValueContainer>* stack;
    
};
'''
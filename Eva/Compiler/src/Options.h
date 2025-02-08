#pragma once
#define DEBUG_TOKENS 0
#define SSA 1
#define DEBUG_SSA 1
#define OPTIMIZATIONS 1
#define DEC OPTIMIZATIONS // dead code elimination
#define CONST_PROP OPTIMIZATIONS  // depends on DEC, DEC cleans up
#define CONSTANT_FOLD OPTIMIZATIONS  // AST folding
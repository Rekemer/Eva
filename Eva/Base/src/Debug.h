#pragma once
#include "Incode.h"
#include "Value.h"
#include "Bytecode.h"
#include <vector>
namespace Eva
{
class HashTable;
void Debug(std::vector<Bytecode>& bytecode,
    std::vector<ValueContainer>& constants, HashTable& globalVariables);
}

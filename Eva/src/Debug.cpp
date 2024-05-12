#include "Debug.h"
#include <iostream>

const char* debugEnum(InCode code);


void Debug(std::vector<Bytecode>& bytecode, std::vector<ValueContainer>& constants)
{
    int ipIndex = 0;
    std::cout << "-------------Debug-----------\n";
    while (ipIndex != bytecode.size() - 1)
    {
        auto instr = (InCode)bytecode[ipIndex++];
        auto str = debugEnum(instr);
        if (instr == InCode::CONST_VALUE)
        {
            auto constant = constants[bytecode[ipIndex++]];
            std::cout << str << std::endl;
            std::cout << constant << std::endl;
        }
        else if (instr == InCode::JUMP  || instr == InCode::JUMP_BACK
            || instr == InCode::JUMP_IF_FALSE)
        {
            std::cout << str << std::endl;
            auto jumpIndexOffset = bytecode[ipIndex++];
            std::cout << static_cast<int>(jumpIndexOffset)<< std::endl;
        }
        else if (instr == InCode::SET_VAR || instr == InCode::GET_VAR)
        {
            std::cout << str << std::endl;
            auto indexOfVariableName = bytecode[ipIndex++];
            std::cout << static_cast<int>(indexOfVariableName)<< std::endl;

        }
        else
        {
            std::cout << str << std::endl;
        }
    }
    std::cout << "-------------Debug-----------\n";
}

const char* debugEnum(InCode code) {
    switch (code) {
    case InCode::CONST_VALUE: return "CONST_VALUE";
    case InCode::TRUE: return "TRUE";
    case InCode::FALSE: return "FALSE";
    case InCode::NIL: return "NIL";
    case InCode::ADD_FLOAT: return "ADD_FLOAT";
    case InCode::ADD_INT: return "ADD_INT";
    case InCode::DIVIDE_INT: return "DIVIDE_INT";
    case InCode::DIVIDE_FLOAT: return "DIVIDE_FLOAT";
    case InCode::MULTIPLY_INT: return "MULTIPLY_INT";
    case InCode::MULTIPLY_FLOAT: return "MULTIPLY_FLOAT";
    case InCode::SUBSTRACT_FLOAT: return "SUBSTRACT_FLOAT";
    case InCode::LESS_FLOAT: return "LESS_FLOAT";
    case InCode::LESS_INT: return "LESS_INT";
    case InCode::GREATER_INT: return "GREATER_INT";
    case InCode::GREATER_FLOAT: return "GREATER_FLOAT";
    case InCode::CAST_FLOAT: return "CAST_FLOAT";
    case InCode::CAST_INT: return "CAST_INT";
    case InCode::SUBSTRACT_INT: return "SUBSTRACT_INT";
    case InCode::INCREMENT_INT: return "INCREMENT_INT";
    case InCode::DECREMENT_INT: return "DECREMENT_INT";
    case InCode::INCREMENT_FLOAT: return "INCREMENT_FLOAT";
    case InCode::DECREMENT_FLOAT: return "DECREMENT_FLOAT";
    case InCode::NEGATE: return "NEGATE";
    case InCode::EQUAL_EQUAL: return "EQUAL_EQUAL";
    case InCode::AND: return "AND";
    case InCode::OR: return "OR";
    case InCode::NOT: return "NOT";
    case InCode::PRINT: return "PRINT";
    case InCode::SET_VAR: return "SET_VAR";
    case InCode::GET_VAR: return "GET_VAR";
    case InCode::JUMP: return "JUMP";
    case InCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
    case InCode::POP: return "POP";
    case InCode::RETURN: return "RETURN";
    case InCode::JUMP_BACK: return "JUMP_BACK";
    default: return "UNKNOWN";
    }
}
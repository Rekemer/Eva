#include "Debug.h"

#include <iostream>

std::string DebugEnum(InCode* code, std::vector<ValueContainer>& constants) {
    switch (*code) {
    case InCode::CONST_VALUE: 
    {

        auto constant = constants[((uint8_t)*code + 1)];
        return "CONST_VALUE ";
    }
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
    default: return "UNKNOWN";
    }
}
void PrintDebug(InCode* opCode, std::vector<ValueContainer>& constants)
{
    auto str = DebugEnum(opCode,constants);
    std::cout << str << std::endl;
}


#include "Debug.h"
#include <iostream>
#include <string>
#include "Function.h"
namespace Eva
{
    const char* debugEnum(InCode code);


    void Debug(std::vector<Bytecode>& bytecode,
        std::vector<ValueContainer>& constants, HashTable& globalVariables)
    {
        int ipIndex = 0;
        std::cout << "-------------Debug-----------\n";
        while (ipIndex < bytecode.size())
        {
            auto instr = (InCode)bytecode[ipIndex++];
            auto str = debugEnum(instr);
            auto isLocal = instr == InCode::SET_LOCAL_VAR || instr == InCode::GET_LOCAL_VAR;
            if (instr == InCode::CONST_VALUE)
            {
                auto constant = constants[bytecode[ipIndex++]];
                std::cout << "[" << ipIndex - 2 << "] " << str << std::endl;
                std::cout << "[" << ipIndex - 1 << "] " << constant << std::endl;
            }
            else if (instr == InCode::JUMP || instr == InCode::JUMP_BACK
                || instr == InCode::JUMP_IF_FALSE)
            {
                std::cout << "[" << ipIndex - 1 << "] " << str << std::endl;
                auto jumpIndexOffset = bytecode[ipIndex++];
                std::cout << "[" << ipIndex - 1 << "] " << static_cast<int>(jumpIndexOffset) << std::endl;
            }
            else if (instr == InCode::CALL)
            {
                std::cout << "[" << ipIndex - 1 << "] " << str << std::endl;
                auto argumentCount = bytecode[ipIndex++];
                std::cout << "[" << ipIndex - 1 << "] " << static_cast<int>(argumentCount) << std::endl;

            }
            else if (instr == InCode::SET_GLOBAL_VAR || instr == InCode::GET_NATIVE_NAME || instr == InCode::GET_GLOBAL_VAR ||
                isLocal)
            {
                std::cout << "[" << ipIndex - 1 << "] " << str << std::endl;
                auto indexOfVariableName = (int)bytecode[ipIndex++];
                // hmmm... can't show local variable name, because there is no stack...
                // show index instead
                if (isLocal)
                {
                    std::cout << "[" << ipIndex - 1 << "] " << indexOfVariableName << std::endl;
                }
                else
                {
                    auto nameOfVariable = constants[indexOfVariableName];
                    std::cout << "[" << ipIndex - 1 << "] " << constants[indexOfVariableName] << std::endl;
                }

            }
            else
            {
                std::cout << "[" << ipIndex - 1 << "] " << str << std::endl;
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
        case InCode::DIVIDE_PERCENT: return "DIVIDE_PERCENT";
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

        case InCode::SET_GLOBAL_VAR: return "SET_GLOBAL_VAR";
        case InCode::GET_GLOBAL_VAR: return "GET_GLOBAL_VAR";
        case InCode::GET_NATIVE_NAME: return "GET_NATIVE_NAME";
        case InCode::SET_LOCAL_VAR: return "SET_LOCAL_VAR";
        case InCode::GET_LOCAL_VAR: return "GET_LOCAL_VAR";

        case InCode::JUMP: return "JUMP";
        case InCode::JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case InCode::POP: return "POP";
        case InCode::RETURN: return "RETURN";
        case InCode::JUMP_BACK: return "JUMP_BACK";
        case InCode::CALL: return "CALL";
        case InCode::STORE_TEMP: return "STORE_TEMP";
        case InCode::LOAD_TEMP: return "LOAD_TEMP";
        default: return "UNKNOWN";
        }
    }
}
#pragma once
#include "Bytecode.h"
namespace Eva
{
	enum class InCode : Bytecode
	{
		CONST_VALUE,
		TRUE,
		FALSE,
		NIL,
		ADD_FLOAT,
		ADD_INT,
		ADD_STRING,
		DIVIDE_INT,
		DIVIDE_FLOAT,
		DIVIDE_PERCENT,


		// not to be in the final code
		MULTIPLY,
		ADD,
		SUBSTRACT,
		DIVIDE,
		//
		MULTIPLY_INT,
		MULTIPLY_FLOAT,
		SUBSTRACT_FLOAT,
		LESS_FLOAT,
		LESS_INT,
		GREATER_INT,
		GREATER_FLOAT,

		CAST_FLOAT,
		CAST_INT,

		CAST_BOOL_FLOAT,
		CAST_BOOL_INT,

		SUBSTRACT_INT,
		INCREMENT_INT,
		DECREMENT_INT,
		INCREMENT_FLOAT,
		DECREMENT_FLOAT,
		NEGATE,
		EQUAL_EQUAL,
		AND,
		OR,
		NOT,


		SET_GLOBAL_VAR,
		GET_GLOBAL_VAR,

		GET_NATIVE_NAME,
		GET_PLUGIN_NAME,

		SET_LOCAL_VAR,
		GET_LOCAL_VAR,

		JUMP,
		JUMP_BACK,

		CALL,

		JUMP_IF_FALSE,
		// used when evaluate if and else statements
		// to clear up stack from condition
		POP,
		RETURN,
		STORE_TEMP,
		LOAD_TEMP,
	};

}

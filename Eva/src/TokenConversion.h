#include"Value.h"
#include"Tokens.h"


inline TokenType TypeToLiteral(ValueType valueType)
{
	switch (valueType)
	{
	case ValueType::INT:
		return TokenType::INT_LITERAL;
		break;
	case ValueType::FLOAT:
		return TokenType::FLOAT_LITERAL;
		break;
	case ValueType::STRING:
		return TokenType::STRING_LITERAL;
		break;
	case ValueType::BOOL:
		return TokenType::BOOL_TYPE;
		break;
	case ValueType::DEDUCE:
		return TokenType::DEDUCE;
		break;
	default:
		assert(false);
		break;
	}
}

inline ValueType LiteralToType(TokenType literalType)
{
	switch (literalType)
	{
	case TokenType::INT_LITERAL:
	case TokenType::INT_TYPE:
		return ValueType::INT;
		break;
	case TokenType::FLOAT_LITERAL:
	case TokenType::FLOAT_TYPE:
		return ValueType::FLOAT;
		break;
	case TokenType::STRING_LITERAL:
	case TokenType::STRING_TYPE:
		return ValueType::STRING;
		break;
	case TokenType::FALSE:
	case TokenType::TRUE:
	case TokenType::BOOL_TYPE:
		return ValueType::BOOL;
		break;
	case TokenType::FUN:
		return ValueType::FUNCTION;
		// += case: identifer has left child as + and right chlid as = which is value to add to the variable 
   //case TokenType::PLUS:
   //	 return ValueType::NIL;
	default:
		assert(false);
		break;
	}
}
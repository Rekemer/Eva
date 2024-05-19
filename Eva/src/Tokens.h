#pragma once
#include <cstdint>
#include <string>
#include <map>
#include "Value.h"
#include <iostream>
enum class TokenType : uint8_t {
	// Single-character tokens.
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS,
	COLON,SEMICOLON, SLASH, STAR,
	// One or two character tokens.
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	PLUS_EQUAL,MINUS_EQUAL,
	SLASH_EQUAL,STAR_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	PLUS_PLUS, MINUS_MINUS,
	
	// Literals.
	IDENTIFIER,INT_LITERAL,FLOAT_LITERAL,STRING_LITERAL,
	
	
	ERROR,
	END,
	
	// Keywords.
	AND, CLASS, ELSE, FALSE,
	FOR, FUN, IF, ELIF, NIL, OR,
	PRINT, RETURN, SUPER, THIS,
	TRUE, VAR, WHILE,
	STRING_TYPE,FLOAT_TYPE,INT_TYPE,BOOL_TYPE,


	// not really a token which lexer produces,
	//  but a way to tell that it is a block of expressions
	BLOCK,
	
};

inline std::tuple<bool, TokenType> IsVariableType(TokenType type)
{
	auto castedType = (int)type;
	for (int i = (int)TokenType::STRING_TYPE; i <= (int)TokenType::BOOL_TYPE; i++)
	{
		if (i == castedType)
		{
			return { true ,static_cast<TokenType>(i)};
		}
	}
	return { false,static_cast<TokenType>(0) };
}
struct Token
{
	Token(TokenType type, ValueContainer&& value, int line) : type{ type }, 
		value{std::move(value)},
		line{line}
	{};
	TokenType type;
	ValueContainer value{};
	int line = -1;
};

inline Token CreateToken(TokenType type, ValueContainer&& value, int line)
{
	Token token{ type,std::move(value), line};
	return token;
}

std::string tokenToString(TokenType token);


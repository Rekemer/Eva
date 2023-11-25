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
	SEMICOLON, SLASH, STAR,
	// One or two character tokens.
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	// Literals.
	IDENTIFIER, STRING, NUMBER,
	// Keywords.
	AND, CLASS, ELSE, FALSE,
	FOR, FUN, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS,
	TRUE, VAR, WHILE,
	ERROR,
	FLOAT,INT,BOOL,
	END
};


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


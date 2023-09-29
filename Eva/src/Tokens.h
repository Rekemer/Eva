#pragma once
#include <cstdint>
#include <string>
#include <map>
#include "Value.h"
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
	END
};


struct Token
{
	Token(TokenType type) : type{type} {};
	Token(TokenType type, ValueContainer value) : type{ type }, value{value} {};
	TokenType type;
	ValueContainer value;
};

inline Token CreateToken(TokenType type, ValueContainer value)
{
	Token token{ type,value};
	return token;
}

std::string tokenToString(TokenType token);


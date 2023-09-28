#pragma once
#include <cstdint>
#include <string>
#include <map>

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
	Token(TokenType type) : type{ type }
	{

	}
	TokenType type;
	float value = 0;
};

inline Token CreateToken(TokenType type, float value)
{
	Token token{ type };
	token.value = value;
	return token;
}

std::string tokenToString(TokenType token);


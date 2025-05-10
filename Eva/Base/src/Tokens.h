#pragma once
#include <cstdint>
#include <string>
#include <map>
#include <iostream>
#include "Value.h"

namespace Eva
{

	enum class TokenType : uint8_t {
		// Single-character tokens.
		LEFT_PAREN, RIGHT_PAREN,
		LEFT_BRACE, RIGHT_BRACE,
		COMMA, DOT, DOUBLE_DOT, MINUS, PLUS,
		COLON, SEMICOLON, SLASH, STAR,
		PERCENT,
		// One or two character tokens.
		BANG, BANG_EQUAL,
		EQUAL, EQUAL_EQUAL,
		PLUS_EQUAL, MINUS_EQUAL,
		SLASH_EQUAL, STAR_EQUAL,
		GREATER, GREATER_EQUAL,
		LESS, LESS_EQUAL,
		PLUS_PLUS, MINUS_MINUS,

		// Literals.
		IDENTIFIER, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL,


		ERROR,
		END,

		// Keywords.
		AND, IMPORT, NULLPTR, CLASS, ELSE, FALSE,
		CONTINUE, BREAK,
		FOR, FUN, IF, ELIF, NIL, OR,
		RETURN, 
		TRUE, VAR, WHILE,
		STRING_TYPE, FLOAT_TYPE, INT_TYPE, PTR_TYPE, BOOL_TYPE,
		

		// not really a token which lexer produces,
		// but a way to tell for example that it is a block of expressions
		// should be reworked, I am too lazy to add other enums
		BLOCK,
		DEDUCE,
		DECLARE,
		// just notift about flow
		JUMP_BRANCH,
		BRANCH,
		BRANCH_ELIF,
		// so we don't hit code generation for if,elif, else branching
		BRANCH_WHILE,
		BRANCH_FOR,
		//jumps to compile
		JUMP,
		JUMP_BACK,
		JUMP_FOR,
		JUMP_WHILE,
		CALL,
		PHI,

		// debugging
		PUSH,

	};
	inline bool IsBinaryBoolOp(TokenType type)
	{
		return type == TokenType::EQUAL_EQUAL ||
			type == TokenType::OR || type == TokenType::AND || type == TokenType::BANG_EQUAL ||
			type == TokenType::GREATER || type == TokenType::GREATER_EQUAL || type == TokenType::LESS ||
			type == TokenType::LESS_EQUAL;
	}
	inline bool IsBinaryOp(TokenType type)
	{
		return type == TokenType::PLUS || type == TokenType::STAR ||
			type == TokenType::SLASH || type == TokenType::PERCENT || type == TokenType::MINUS || IsBinaryBoolOp(type);
	}
	inline std::tuple<bool, TokenType> IsVariableType(TokenType type)
	{
		auto castedType = (int)type;
		for (int i = (int)TokenType::STRING_TYPE; i <= (int)TokenType::BOOL_TYPE; i++)
		{
			if (i == castedType)
			{
				return { true ,static_cast<TokenType>(i) };
			}
		}
		return { false,static_cast<TokenType>(0) };
	}
	struct Token
	{
		Token(TokenType type, ValueContainer&& value, int line) : type{ type },
			value{ std::move(value) },
			line{ line }
		{};
		TokenType type;
		ValueContainer value{};
		int line = -1;
	};

	inline Token CreateToken(TokenType type, ValueContainer&& value, int line)
	{
		Token token{ type,std::move(value), line };
		return token;
	}

	std::string tokenToString(TokenType token);

}
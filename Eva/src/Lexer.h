#pragma once
#include"Tokens.h"
#include <vector>
// lexer - produces tokens
class Lexer
{
public:
	Lexer()
	{
		tokens.reserve(100);
	}
	bool Parse(const char* source);
	 std::vector<Token>& GetTokens() { return tokens; };
private:
	void ParseOperator();
	void ParseNumber();
	void ParseAlpha();
	void ParseString();
	void ParseBool();
	void Eat();
	void EatWhiteSpace();
	char Peek(int offset = 0);
private:
	std::vector<Token> tokens;
	bool panic;
	Token* currentToken;
	const char* startSymbol = nullptr;
	const char* currentSymbol = nullptr;
};
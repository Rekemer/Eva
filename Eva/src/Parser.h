#pragma once
#include"Tokens.h"
#include <vector>
class Parser
{
public:
	void Parse(const char* source);
	const std::vector<Token>& GetTokens() { return tokens; };
private:
	void ParseOperator();
	void ParseNumber();
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
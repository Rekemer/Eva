#pragma once
#include"Tokens.h"
#include <vector>
class VirtualMachine;
class String;
// lexer - produces tokens
class Lexer
{
public:
	Lexer()
	{
		tokens.reserve(100);
	}
	bool Parse(const char* source, VirtualMachine& vm);
	 std::vector<Token>& GetTokens() { return tokens; };
private:
	void ParseOperator();
	void ParseNumber();
	void ParseAlpha();
	void ParseString(VirtualMachine& vm);
	void ParseBool();
	void ParseDeclaration(VirtualMachine& vm);
	bool IsEndExpression();
	void ParseStatement();
	void Eat();
	void EatWhiteSpace();
	void EatComments();
	// eats statement and declaration tokens
	void EatType(TokenType type);

	void ErrorCharacter(const char* msg, const char character, size_t line);
	void Error(const char* msg,  size_t line);
	char Peek(int offset = 0);
	void AddToken(TokenType type, int line);
	bool IsPartOfVariable(char c);
private:
	std::vector<Token> tokens;
	bool panic;
	Token* currentToken;
	const char* startSymbol = nullptr;
	const char* currentSymbol = nullptr;
	size_t currentLine = 1;
};
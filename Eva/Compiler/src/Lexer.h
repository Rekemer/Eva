#pragma once
#include"Tokens.h"
#include <vector>
#include <iterator>
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
	bool Parse(const char* source);
	 std::vector<Token>& GetTokens() { return tokens; };
private:
	bool IsNewLine(const char* symbol);
	void ParseOperator();
	void ParseNumber();
	void ParseAlpha();
	void ParseString();
	void ParseBool();
	void ParseDeclaration();
	void ParseStatement();
	void Eat();
	void EatWhiteSpace();
	void EatOdinaryComments();
	void EatDoubleComments();
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
	const char* endFile = nullptr;
	const char* currentSymbol = nullptr;
	size_t currentLine = 1;
};
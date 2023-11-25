#include"Lexer.h"
#include <iostream>
#include"String.hpp"
#include"VirtualMachine.h"




void Lexer::ErrorCharacter(const char* msg ,const char character, size_t line)
{
	panic = true;
	std::cout <<"Lexer Error [" << line << "] " << msg << character << std::endl;
}
void Lexer::Error(const char* msg, size_t line)
{
	panic = true;
	std::cout << "Lexer Error [" << line << "] " << msg  << std::endl;
}

bool IsMatch(const char* str,TokenType type)
{
	switch (type)
	{
	case TokenType::LEFT_PAREN:
		break;
	case TokenType::RIGHT_PAREN:
		break;
	case TokenType::LEFT_BRACE:
		break;
	case TokenType::RIGHT_BRACE:
		break;
	case TokenType::COMMA:
		break;
	case TokenType::DOT:
		break;
	case TokenType::MINUS:
		break;
	case TokenType::PLUS:
		break;
	case TokenType::SEMICOLON:
		break;
	case TokenType::SLASH:
		break;
	case TokenType::STAR:
		break;
	case TokenType::BANG:
		break;
	case TokenType::BANG_EQUAL:
		break;
	case TokenType::EQUAL:
		break;
	case TokenType::EQUAL_EQUAL:
		break;
	case TokenType::GREATER:
		break;
	case TokenType::GREATER_EQUAL:
		break;
	case TokenType::LESS:
		break;
	case TokenType::LESS_EQUAL:
		break;
	case TokenType::IDENTIFIER:
		break;
	case TokenType::STRING:
		return memcmp(str, "String", 6) == 0;
		break;
	case TokenType::NUMBER:
		break;
	case TokenType::AND:
		break;
	case TokenType::CLASS:
		break;
	case TokenType::ELSE:
		break;
	case TokenType::FALSE:
		return memcmp(str, "false", 5) == 0;
		break;
	case TokenType::FOR:
		break;
	case TokenType::FUN:
		break;
	case TokenType::IF:
		break;
	case TokenType::NIL:
		break;
	case TokenType::OR:
		break;
	case TokenType::PRINT:
		return memcmp(str, "Print", 5) == 0;
		break;
	case TokenType::RETURN:
		break;
	case TokenType::SUPER:
		break;
	case TokenType::THIS:
		break;
	case TokenType::TRUE:
		return memcmp(str, "true", 4) == 0;
		break;
	case TokenType::VAR:
		break;
	case TokenType::WHILE:
		break;
	case TokenType::ERROR:
		break;
	case TokenType::INT:
		return memcmp(str, "int", 3) == 0;
		break;
	case TokenType::FLOAT:
		return memcmp(str, "float", 5) == 0;
		break;
	case TokenType::END:
		break;
	default:
		return false;
		break;
	}
}

// eat comments too?
void Lexer::EatWhiteSpace()
{
	bool isRunning = true;
	while (isRunning)
	{

		switch (*currentSymbol) {
		case ' ':
		case '\r':
		case '\t':
			currentSymbol++;
			break;
		case '\n':
		{
			currentSymbol++;
			currentLine++;
			break;
		}
		default:
			isRunning = false;
			break;
		}
	}
	startSymbol = currentSymbol;
}


 char Lexer::Peek(int offset)
{
	return *(currentSymbol+ offset);
}

void Lexer::Eat()
{
	currentSymbol++;
}

void Lexer::ParseString(VirtualMachine& vm)
{
	EatWhiteSpace();
	if (Peek() == '"')
	{
		Eat();
		ParseAlpha();
		if (Peek() =='\"')
		{
			auto size = static_cast<size_t>(currentSymbol - startSymbol);
			auto* obj = vm.AllocateString(startSymbol,size);
			tokens.push_back(CreateToken(TokenType::STRING, ValueContainer{ obj }, currentLine));
			tokens.back().line = currentLine;
			Eat();
		}
		else
		{
			Error("Unterminated String",currentLine);
		}
	}
}
void Lexer::ParseBool()
{
	EatWhiteSpace();
	if (IsMatch(currentSymbol,TokenType::TRUE))
	{
		tokens.push_back(CreateToken(TokenType::TRUE, ValueContainer{ true }, currentLine));
		currentSymbol += 4;
	}
	else if (IsMatch(currentSymbol, TokenType::FALSE))
	{
		tokens.push_back(CreateToken(TokenType::FALSE, ValueContainer{ false }, currentLine));
		currentSymbol += 5;
	}
	startSymbol = currentSymbol;
}
// parse sequence of characters
void Lexer::ParseAlpha()
{
	EatWhiteSpace();

	//auto isAlpha = [](char c) {return c >= 48 && c <= 'Z' || c>='a' && c <= 'z' || c == '/'; };
	auto isPartOfString = [](char c) {return c >= 32 && c <= 126 && c != '"'; };
	if (isPartOfString(Peek()))
	{
		while (isPartOfString(Peek()))
		{
			Eat();
		}
	}
}

void Lexer::ParseNumber()
{
	EatWhiteSpace();
	// check if number
	auto isDigit = [](char symbol) {return symbol >= '0' && symbol <= '9'; };
	if (isDigit(Peek()))
	{
		// parse a number
		while (isDigit(Peek()))
		{
			Eat();
			if (*currentSymbol == '.' && isDigit(Peek(1)))
			{
				Eat();
				while (isDigit(Peek()))
				{
					Eat();
				}
			}
		}
		float floatValue = std::strtof(startSymbol, nullptr);
		tokens.push_back(CreateToken(TokenType::NUMBER, ValueContainer{floatValue}, currentLine));
		startSymbol = currentSymbol;
	}
}


void Lexer::AddToken(TokenType type, int line)
{
	tokens.emplace_back(CreateToken(type, {}, line));
}

void Lexer::ParseOperator()
{
	// check if operator
	EatWhiteSpace();
	
	if (Peek() == '\0') return;
	auto currentCharacter = Peek();
	switch (currentCharacter)
	{
	case '(':

	{
		AddToken(TokenType::LEFT_PAREN, currentLine);
		break;

	}
	case ')':
	{
		AddToken(TokenType::RIGHT_PAREN, currentLine);
		break;
	};
	case '{':
	{
		AddToken(TokenType::LEFT_BRACE, currentLine);
		break;
	}
	case '}':
	{
		AddToken(TokenType::RIGHT_BRACE, currentLine);
		break;
	}
	case ';':
	{
		AddToken(TokenType::SEMICOLON, currentLine);
		break;
	}
	case ',':
	{
		AddToken(TokenType::COMMA, currentLine);
		break;
	}
	case '.':
	{
		AddToken(TokenType::DOT, currentLine);
		break;
	}
	case '-':
	{
		AddToken(TokenType::MINUS, currentLine);
		break;
	}
	case '+':
	{
		AddToken(TokenType::PLUS, currentLine);
		break;
	}
	case '/':
	{
		AddToken(TokenType::SLASH, currentLine);
		break;
	}
	case '*':
	{
		AddToken(TokenType::STAR, currentLine);
		break;
	}
	case '!':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{	
			AddToken(TokenType::BANG_EQUAL, currentLine);
			Eat();
		}
		else
		{	
			AddToken(TokenType::BANG, currentLine);
		}
		break;
	}
	case '>':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::GREATER_EQUAL, currentLine);
			Eat();
		}
		else
		{
			AddToken(TokenType::GREATER, currentLine);

		}
			break;
	}
	case '<':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::LESS_EQUAL, currentLine);
			Eat();
		}
		else
		{
			AddToken(TokenType::LESS, currentLine);
		}
		break;
	}
	case '=':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::EQUAL_EQUAL, currentLine);
			Eat();
		}
		else
		{
			AddToken(TokenType::EQUAL, currentLine);
		}
		break;
	}
	case '&':
	{
		if (Peek(1) != '\0' && Peek(1) == '&')
		{
			AddToken(TokenType::AND, currentLine);
			Eat();
		}
		break;
	}
	case '|':
	{
		if (Peek(1) != '\0' && Peek(1) == '|')
		{
			AddToken(TokenType::OR, currentLine);
			Eat();
		}
		break;
	}
	default:
	{
		ErrorCharacter("unexpected character ",currentCharacter, currentLine);
		break;
	}
	}

	if (Peek() != '\0')
	{
		Eat();

	}
}
void Lexer::ParseDeclaration()
{
	if (IsMatch(startSymbol, TokenType::PRINT))
	{	
		tokens.emplace_back(TokenType::PRINT, ValueContainer{}, currentLine);
		currentSymbol += 5;
	}
	else if (IsMatch(startSymbol, TokenType::INT))
	{
		tokens.emplace_back(TokenType::INT, ValueContainer{}, currentLine);
		currentSymbol += 3;
	}
	else if (IsMatch(startSymbol, TokenType::FLOAT))
	{
		tokens.emplace_back(TokenType::FLOAT, ValueContainer{}, currentLine);
		currentSymbol += 5;
	}
	else if (IsMatch(startSymbol, TokenType::STRING))
	{
		tokens.emplace_back(TokenType::STRING, ValueContainer{}, currentLine);
		currentSymbol += 6;
	}
	
}
bool Lexer::Parse(const char* source, VirtualMachine& vm)
{
	startSymbol = source;
	panic = false;
	currentSymbol = source;
	while (*currentSymbol != '\0')
	{
		EatWhiteSpace();


		ParseDeclaration();
		ParseString(vm);
		ParseBool();
		ParseNumber();
		ParseOperator();
		if (panic)
		{
			std::cout << "---------------------------------------\n";
			std::cout << "PANIC!\n";
			std::cout << "---------------------------------------\n";
			// deal with errors

			return false;
			break;
		}
	}
	tokens.emplace_back(TokenType::END, ValueContainer{},currentLine);
	return true;
}

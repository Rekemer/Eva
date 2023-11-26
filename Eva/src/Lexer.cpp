#include"Lexer.h"
#include <iostream>
#include"String.hpp"
#include"VirtualMachine.h"
#include <cassert>


bool IsPartOfString (char c) {return c >= 32 && c <= 126 && c != '"'; };

bool IsPartOfVariable(char c)
{
	return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; 
};

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


bool IsMatch(const char* str, size_t strSize, TokenType type) {

	switch (type) {
	case TokenType::LEFT_PAREN:
	case TokenType::RIGHT_PAREN:
	case TokenType::LEFT_BRACE:
	case TokenType::RIGHT_BRACE:
	case TokenType::COMMA:
	case TokenType::DOT:
	case TokenType::MINUS:
	case TokenType::PLUS:
	case TokenType::SEMICOLON:
	case TokenType::SLASH:
	case TokenType::STAR:
	case TokenType::BANG:
	case TokenType::BANG_EQUAL:
	case TokenType::EQUAL:
	case TokenType::EQUAL_EQUAL:
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
	case TokenType::LESS:
	case TokenType::LESS_EQUAL:
	case TokenType::AND:
	case TokenType::CLASS:
	case TokenType::ELSE:
	case TokenType::FOR:
	case TokenType::FUN:
	case TokenType::IF:
	case TokenType::NIL:
	case TokenType::OR:
	case TokenType::RETURN:
	case TokenType::SUPER:
	case TokenType::THIS:
	case TokenType::VAR:
	case TokenType::WHILE:
	case TokenType::ERROR:
	case TokenType::END:
		return false; 


	case TokenType::STRING_TYPE:
		return strSize == 6 && String::AreEqual(str, strSize, "String", 6);

	case TokenType::FALSE:
		return strSize == 5 && String::AreEqual(str, strSize, "false", 5);

	case TokenType::TRUE:
		return strSize == 4 && String::AreEqual(str, strSize, "true", 4);

	case TokenType::PRINT:
		return strSize == 5 && String::AreEqual(str, strSize, "Print", 5);

	case TokenType::INT_TYPE:
		return strSize == 3 && String::AreEqual(str, strSize, "int", 3);

	case TokenType::FLOAT_TYPE:
		return strSize == 5 && String::AreEqual(str, strSize, "float", 5);

	default:
		return false;
	}
}



void Lexer::EatType(TokenType type)
{
	if (type == TokenType::PRINT)
	{
		tokens.emplace_back(CreateToken(TokenType::PRINT, ValueContainer{}, currentLine));
		currentSymbol += 5;
	}
	else if (type == TokenType::INT_TYPE)
	{
		tokens.emplace_back(CreateToken(TokenType::INT_TYPE, ValueContainer{}, currentLine));
		currentSymbol += 3;
	}
	else if (type == TokenType::FLOAT_TYPE)
	{
		tokens.emplace_back(CreateToken(TokenType::FLOAT_TYPE, ValueContainer{}, currentLine));
		currentSymbol += 5;
	}
	else if (type == TokenType::STRING_TYPE)
	{
		tokens.emplace_back(CreateToken(TokenType::STRING_TYPE, ValueContainer{}, currentLine));
		currentSymbol += 6;
	}
	else
	{
		assert(false);
	}
	
}
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
			tokens.push_back(CreateToken(TokenType::STRING_LITERAL, ValueContainer{ obj }, currentLine));
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
	if (IsMatch(currentSymbol,4,TokenType::TRUE))
	{
		tokens.push_back(CreateToken(TokenType::TRUE, ValueContainer{ true }, currentLine));
		currentSymbol += 4;
	}
	else if (IsMatch(currentSymbol,5, TokenType::FALSE))
	{
		tokens.push_back(CreateToken(TokenType::FALSE, ValueContainer{ false }, currentLine));
		currentSymbol += 5;
	}
}
// parse sequence of characters
void Lexer::ParseAlpha()
{
	EatWhiteSpace();
	if (IsPartOfString(Peek()))
	{
		while (IsPartOfString(Peek()))
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
		bool isFloat = false;
		// parse a number
		while (isDigit(Peek()))
		{
			Eat();
			if (*currentSymbol == '.' && isDigit(Peek(1)))
			{
				isFloat = true;
				Eat();
				while (isDigit(Peek()))
				{
					Eat();
				}
			}
		}
		if (isFloat)
		{
			float floatValue = std::strtof(startSymbol, nullptr);
			tokens.push_back(CreateToken(TokenType::FLOAT_LITERAL, ValueContainer{floatValue}, currentLine));
		}
		else
		{
			auto intValue = atoi(startSymbol);
			tokens.push_back(CreateToken(TokenType::INT_LITERAL, ValueContainer{ intValue }, currentLine));
		}
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
		Eat();
		break;

	}
	case ')':
	{
		AddToken(TokenType::RIGHT_PAREN, currentLine);
		Eat();
		break;
	};
	case '{':
	{
		AddToken(TokenType::LEFT_BRACE, currentLine);
		Eat();
		break;
	}
	case '}':
	{
		AddToken(TokenType::RIGHT_BRACE, currentLine);
		Eat();
		break;
	}
	case ':':
	{
		AddToken(TokenType::COLON, currentLine);
		Eat();
		break;
	}
	case ';':
	{
		AddToken(TokenType::SEMICOLON, currentLine);
		Eat();
		startSymbol = currentSymbol;
		break;
	}
	case ',':
	{
		AddToken(TokenType::COMMA, currentLine);
		Eat();
		break;
	}
	case '.':
	{
		AddToken(TokenType::DOT, currentLine);
		Eat();
		break;
	}
	case '-':
	{
		AddToken(TokenType::MINUS, currentLine);
		Eat();
		break;
	}
	case '+':
	{
		AddToken(TokenType::PLUS, currentLine);
		Eat();
		break;
	}
	case '/':
	{
		AddToken(TokenType::SLASH, currentLine);
		Eat();
		break;
	}
	case '*':
	{
		AddToken(TokenType::STAR, currentLine);
		Eat();
		break;
	}
	case '!':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{	
			AddToken(TokenType::BANG_EQUAL, currentLine);
			Eat();
			Eat();
		}
		else
		{	
			AddToken(TokenType::BANG, currentLine);
			Eat();
		}
		break;
	}
	case '>':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::GREATER_EQUAL, currentLine);
			Eat();
			Eat();
		}
		else
		{
			AddToken(TokenType::GREATER, currentLine);
			Eat();
		}
			break;
	}
	case '<':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::LESS_EQUAL, currentLine);
			Eat();
			Eat();
		}
		else
		{
			AddToken(TokenType::LESS, currentLine);
			Eat();
		}
		break;
	}
	case '=':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			AddToken(TokenType::EQUAL_EQUAL, currentLine);
			Eat();
			Eat();
		}
		else
		{
			AddToken(TokenType::EQUAL, currentLine);
			Eat();
		}
		break;
	}
	case '&':
	{
		if (Peek(1) != '\0' && Peek(1) == '&')
		{
			AddToken(TokenType::AND, currentLine);
			Eat();
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
			Eat();
		}
		break;
	}
	default:
	{
		break;
	}
	}

	
}


bool Lexer::IsEndExpression()
{
	EatWhiteSpace();
	return Peek() == ';';
}
// a variable, a function
void Lexer::ParseDeclaration(VirtualMachine& vm)
{
	EatWhiteSpace();
	
	if (IsPartOfVariable(Peek()))
	{
		while (IsPartOfVariable(Peek()))
		{
			Eat();
		}
		size_t size = currentSymbol - startSymbol;
		// iterate over keywords
		for (int i = (int)TokenType::AND; i < (int)TokenType::BOOL_TYPE; i++)
		{
			auto isKeyword = IsMatch(startSymbol, size,static_cast<TokenType>(i));
			if (isKeyword)
			{
				tokens.push_back(CreateToken(static_cast<TokenType>(i), {}, currentLine));
				return;
			}
		}
		auto* variableName = vm.AllocateString(startSymbol,size);
		tokens.push_back(CreateToken(TokenType::IDENTIFIER, ValueContainer{ variableName }, currentLine));
	}
	
	

}
void Lexer::ParseStatement()
{
	if (IsMatch(currentSymbol,5, TokenType::PRINT))
	{	
		EatType(TokenType::PRINT);
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
		
		startSymbol = currentSymbol;
		ParseDeclaration(vm);
		
 		ParseStatement();
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

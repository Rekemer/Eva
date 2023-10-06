#include"Lexer.h"
#include <iostream>
#include"String.hpp"
// eat comments too?
void Lexer::EatWhiteSpace()
{
	while (true)
	{

		switch (*currentSymbol) {
		case ' ':
		case '\r':
		case '\t':
			currentSymbol++;
			break;
		case '\n':
			currentSymbol++;
			break;
		default:
			return;
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

void Lexer::ParseString()
{
	if (Peek() == '"')
	{
		Eat();
		ParseAlpha();
		if (Peek() =='\"')
		{
			auto size = static_cast<size_t>(currentSymbol - startSymbol - 1);
			Object* obj = new String{ startSymbol+1,size };
			tokens.push_back(CreateToken(TokenType::STRING, (ValueContainer{ obj })));
			Eat();
		}
	}
}
void Lexer::ParseBool()
{
	if (memcmp(startSymbol, "true", 4) == 0)
	{
		tokens.push_back(CreateToken(TokenType::TRUE, ValueContainer{ true }));
		currentSymbol += 4;
	}
	else if (memcmp(startSymbol, "false", 5) == 0)
	{
		tokens.push_back(CreateToken(TokenType::FALSE, ValueContainer{ false }));
		currentSymbol += 5;
	}
	startSymbol = currentSymbol;
}
// parse variables
void Lexer::ParseAlpha()
{
	// parse string
	

	auto isAlpha = [](char c) {return c >= 'A' && c <= 'Z' || c>='a' && c <= 'z' ||c == '/'; };
	
	if (isAlpha(Peek()))
	{
		while (isAlpha(Peek()))
		{
			Eat();
		}
	}
}

void Lexer::ParseNumber()
{

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
		tokens.push_back(CreateToken(TokenType::NUMBER, ValueContainer{floatValue}));
		startSymbol = currentSymbol;
	}
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
		tokens.emplace_back(TokenType::LEFT_PAREN);
		break;

	}
	case ')':
	{
		tokens.emplace_back(TokenType::RIGHT_PAREN);
		break;
	};
	case '{':
	{
		tokens.emplace_back(TokenType::LEFT_BRACE);
		break;
	}
	case '}':
	{
		tokens.emplace_back(TokenType::RIGHT_BRACE);
		break;
	}
	case ';':
	{
		tokens.emplace_back(TokenType::SEMICOLON);
		break;
	}
	case ',':
	{
		tokens.emplace_back(TokenType::COMMA);
		break;
	}
	case '.':
	{
		tokens.emplace_back(TokenType::DOT);
		break;
	}
	case '-':
	{
		tokens.emplace_back(TokenType::MINUS);
		break;
	}
	case '+':
	{
		tokens.emplace_back(TokenType::PLUS);
		break;
	}
	case '/':
	{
		tokens.emplace_back(TokenType::SLASH);
		break;
	}
	case '*':
	{
		tokens.emplace_back(TokenType::STAR);
		break;
	}
	case '!':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			tokens.emplace_back(TokenType::BANG_EQUAL);
			Eat();
		}
		else
		{
			tokens.emplace_back(TokenType::BANG);
		}
		break;
	}
	case '>':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			tokens.emplace_back(TokenType::GREATER_EQUAL);
			Eat();
		}
		else
		{
			tokens.emplace_back(TokenType::GREATER);

		}
			break;
	}
	case '<':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			tokens.emplace_back(TokenType::LESS_EQUAL);
			Eat();
		}
		else
		{
			tokens.emplace_back(TokenType::LESS);
		}
		break;
	}
	case '=':
	{
		if (Peek(1) != '\0' && Peek(1) == '=')
		{
			tokens.emplace_back(TokenType::EQUAL_EQUAL);
			Eat();
		}
		else
		{
			tokens.emplace_back(TokenType::EQUAL);
		}
		break;
	}
	default:
	{
		panic = true;
		std::cout << "LEXER ERROR: unexpected character " << currentCharacter <<"\n";
		break;
	}
	}

	if (Peek() != '\0')
	{
		Eat();

	}
}

bool Lexer::Parse(const char* source)
{
	startSymbol = source;
	panic = false;
	currentSymbol = source;
	while (*currentSymbol != '\0')
	{
		EatWhiteSpace();

		startSymbol = currentSymbol;

		ParseString();
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
	tokens.emplace_back(TokenType::END);
	return true;
}

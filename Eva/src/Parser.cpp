#include"Parser.h"
#include <iostream>

// eat comments too?
void Parser::EatWhiteSpace()
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


 char Parser::Peek(int offset)
{
	return *(currentSymbol+ offset);
}

void Parser::Eat()
{
	currentSymbol++;
}

void Parser::ParseNumber()
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
		tokens.push_back(CreateToken(TokenType::NUMBER, floatValue));
	}
}

void Parser::ParseOperator()
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

			break;
		}
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
	default:
	{
		panic = true;
		std::cout << "unexpected character " << currentCharacter <<"\n";
		break;
	}
	}

	if (Peek() != '\0')
	{
		Eat();

	}
}

void Parser::Parse(const char* source)
{
	startSymbol = source;
	panic = false;
	currentSymbol = source;
	while (*currentSymbol != '\0')
	{
		EatWhiteSpace();

		startSymbol = currentSymbol;

		ParseNumber();
		ParseOperator();
		if (panic)
		{
			break;
		}

	}
	tokens.emplace_back(TokenType::END);
}

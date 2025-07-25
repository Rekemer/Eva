#include"Lexer.h"
#include <iostream>
#include <string>
#include <cassert>

namespace Eva
{
	bool IsPartOfString(const char  c) { return c >= 32 && c <= 126 && c != '"'; };
	bool IsDigit(const char  symbol)
	{
		return symbol >= '0' && symbol <= '9';
	};
	bool IsCharacter(const char  c)
	{
		return  c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z' || c == '_';
	}
	bool Lexer::IsPartOfVariable(const char c)
	{
		if (IsDigit(c))
		{
			bool hasVariableStarted = IsCharacter(*startSymbol);
			return hasVariableStarted;
		}
		return IsCharacter(c);
	};

	void Lexer::ErrorCharacter(const char* msg, const char character, size_t line)
	{
		panic = true;
		std::cout << "Lexer Error [" << line << "] " << msg << character << std::endl;
	}
	void Lexer::Error(const char* msg, size_t line)
	{
		panic = true;
		std::cout << "Lexer Error [" << line << "] " << msg << std::endl;
	}

	bool AreEqual(const char* str, size_t size, const char* str2, size_t size2)
	{
		if (size != size2) return false;
		return !static_cast<bool>(strncmp(str, str2, size));
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
		case TokenType::NIL:
		case TokenType::OR:
		case TokenType::ERROR:
		case TokenType::END:
			return false;

		case TokenType::RETURN:
			return strSize == 6 && AreEqual(str, strSize, "return", 6);
		case TokenType::FUN:
			return strSize == 3 && AreEqual(str, strSize, "fun", 3);

		case TokenType::STRING_TYPE:
			return strSize == 6 && AreEqual(str, strSize, "String", 6);

		case TokenType::FALSE:
			return strSize == 5 && AreEqual(str, strSize, "false", 5);

		case TokenType::TRUE:
			return strSize == 4 && AreEqual(str, strSize, "true", 4);
		case TokenType::CONTINUE:
			return strSize == 8 && AreEqual(str, strSize, "continue", 8);
		case TokenType::IMPORT:
			return strSize == 6 && AreEqual(str, strSize, "import", 6);

		case TokenType::BREAK:
			return strSize == 5 && AreEqual(str, strSize, "break", 5);
		case TokenType::IF:
			return strSize == 2 && AreEqual(str, strSize, "if", 2);
		case TokenType::ELIF:
			return strSize == 4 && AreEqual(str, strSize, "elif", 4);
		case TokenType::ELSE:
			return strSize == 4 && AreEqual(str, strSize, "else", 4);
		case TokenType::WHILE:
			return strSize == 5 && AreEqual(str, strSize, "while", 5);
		case TokenType::FOR:
			return strSize == 3 && AreEqual(str, strSize, "for", 3);
		case TokenType::INT_TYPE:
			return strSize == 3 && AreEqual(str, strSize, "int", 3);
		case TokenType::FLOAT_TYPE:
			return strSize == 5 && AreEqual(str, strSize, "float", 5);
		case TokenType::PTR_TYPE:
			return strSize == 7 && AreEqual(str, strSize, "pointer", 7);
		case TokenType::NULLPTR:
			return strSize == 7 && AreEqual(str, strSize, "nullptr", 7);
		case TokenType::BOOL_TYPE:
			return strSize == 4 && AreEqual(str, strSize, "bool", 4);
		default:
			return false;
		}
	}

	bool Lexer::IsNewLine(const char* symbol)
	{
		if (Peek() == '\0') return false;
		if (Peek(1) == '\0') return false;
		return Peek() == '/' && Peek(1) == 'n';
	}
	void Lexer::EatDoubleComments()
	{
		auto isCommentEatable = [&]()
			{
				auto isStar = Peek() != '\0' && Peek() != '*';
				auto isStar1 = Peek(1) != '\0' && Peek(1) != '/';
				if (Peek() == '\0') return false;
				if (Peek(1) == '\0') return false;
				auto isComment = Peek() == '*' && Peek(1) == '/';

				return (!isComment || IsNewLine(currentSymbol + 1));
			};
		while (isCommentEatable())
		{
			if (IsNewLine(currentSymbol))
			{
				currentLine++;
			}
			currentSymbol++;
		}
		assert(Peek() != '\0' && Peek() == '*'
			&& Peek(1) != '\0' && Peek(1) == '/');
		if ((*currentSymbol) != '\0')
			currentSymbol += 2;
	}

	void Lexer::EatOdinaryComments()
	{
		while (Peek() != '\n' && Peek() != '\0')
		{
			currentSymbol++;
		}
		if ((*currentSymbol) != '\0')
			currentSymbol++;
		currentLine++;
	}
	void Lexer::EatWhiteSpace()
	{
		bool isRunning = true;
		while (isRunning)
		{
			bool isComment = (*currentSymbol) == '/' && (*(currentSymbol + 1)) == '/';
			bool isCommentDouble = (*currentSymbol) == '/' && (*(currentSymbol + 1)) == '*';
			//std::cout << "--------------------------------------" << std::endl;
			//std::cout << currentSymbol << std::endl;
			if (isComment)
			{
				EatOdinaryComments();
			}
			else if (isCommentDouble)
			{
				currentSymbol += 2;
				EatDoubleComments();
			}
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
			{

				bool isCommentOdinary = (*currentSymbol) == '/' && (*(currentSymbol + 1)) == '/';
				bool isCommentDouble = (*currentSymbol) == '/' && (*(currentSymbol + 1)) == '*';
				if (isCommentOdinary)
				{
					EatOdinaryComments();
				}
				else if (isCommentDouble)
				{
					currentSymbol += 2;
					EatDoubleComments();
				}
				else
				{
					isRunning = false;
				}
			}
			break;
			}
		}


	}


	char Lexer::Peek(int offset)
	{
		return *(currentSymbol + offset);
	}

	void Lexer::Eat()
	{
		currentSymbol++;
	}

	void Lexer::ParseString()
	{
		EatWhiteSpace();
		if (Peek() == '"')
		{
			Eat();
			startSymbol = currentSymbol;
			ParseAlpha();
			if (Peek() == '\"')
			{
				auto size = static_cast<size_t>(currentSymbol - startSymbol);
				auto obj = std::string{ startSymbol, size };
				tokens.push_back(CreateToken(TokenType::STRING_LITERAL, ValueContainer{ obj }, currentLine));
				Eat();
			}
			else
			{
				Error("Unterminated String", currentLine);
			}
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
		if (IsDigit(Peek()))
		{
			bool isFloat = false;
			auto nextToken = currentSymbol;
			// parse a number
			while (IsDigit(Peek()))
			{
				Eat();
				if (*currentSymbol == '.' && IsDigit(Peek(1)))
				{
					isFloat = true;
					Eat();
					while (IsDigit(Peek()))
					{
						Eat();
					}
				}
			}
			if (isFloat)
			{
				efloat floatValue = std::strtod(nextToken, nullptr);
				tokens.push_back(CreateToken(TokenType::FLOAT_LITERAL, ValueContainer{ floatValue }, currentLine));
			}
			else
			{
				eint intValue = atoi(nextToken);
				tokens.push_back(CreateToken(TokenType::INT_LITERAL, ValueContainer{ intValue }, currentLine));
			}
			//startSymbol = currentSymbol;
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
			if (Peek(1) != '\0' && Peek(1) == '.')
			{
				AddToken(TokenType::DOUBLE_DOT, currentLine);
				Eat();
				Eat();
			}
			else
			{
				AddToken(TokenType::DOT, currentLine);
				Eat();
			}
			break;
		}
		case '-':
		{
			if (Peek(1) != '\0' && Peek(1) == '=')
			{
				AddToken(TokenType::MINUS_EQUAL, currentLine);
				Eat();
			}
			else if (Peek(1) != '\0' && Peek(1) == '-')
			{
				AddToken(TokenType::MINUS_MINUS, currentLine);
				Eat();
			}
			else
			{
				AddToken(TokenType::MINUS, currentLine);
			}
			Eat();

			break;
		}
		case '+':
		{
			if (Peek(1) != '\0' && Peek(1) == '=')
			{
				AddToken(TokenType::PLUS_EQUAL, currentLine);
				Eat();
			}
			else if (Peek(1) != '\0' && Peek(1) == '+')
			{
				AddToken(TokenType::PLUS_PLUS, currentLine);
				Eat();
			}
			else
			{
				AddToken(TokenType::PLUS, currentLine);
			}
			Eat();
			break;
		}
		case '/':
		{
			if (Peek(1) != '\0' && Peek(1) == '=')
			{
				AddToken(TokenType::SLASH_EQUAL, currentLine);
				Eat();
			}
			else
			{

				AddToken(TokenType::SLASH, currentLine);
			}
			Eat();
			break;
		}
		case '*':
		{
			if (Peek(1) != '\0' && Peek(1) == '=')
			{
				AddToken(TokenType::STAR_EQUAL, currentLine);
				Eat();
			}
			else
			{

				AddToken(TokenType::STAR, currentLine);
			}
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
		case '%':
		{
			AddToken(TokenType::PERCENT, currentLine);
			Eat();
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



	// a variable, a function and keywords like print and if
	void Lexer::ParseDeclaration()
	{
		EatWhiteSpace();
		startSymbol = currentSymbol;
		if (IsPartOfVariable(Peek()))
		{
			while (IsPartOfVariable(Peek()))
			{
				Eat();
			}
			size_t size = currentSymbol - startSymbol;
			// iterate over keywords
			for (int i = (int)TokenType::AND; i <= (int)TokenType::BOOL_TYPE; i++)
			{
				auto isKeyword = IsMatch(startSymbol, size, static_cast<TokenType>(i));
				if (isKeyword)
				{
					if (static_cast<TokenType>(i) == TokenType::TRUE)
					{
						tokens.push_back(CreateToken(TokenType::TRUE, ValueContainer{ true }, currentLine));
					}
					else if (static_cast<TokenType>(i) == TokenType::FALSE)
					{
						tokens.push_back(CreateToken(TokenType::FALSE, ValueContainer{ false }, currentLine));
					}
					else
					{
						tokens.push_back(CreateToken(static_cast<TokenType>(i), {}, currentLine));

					}
					startSymbol = currentSymbol;
					return;
				}
			}
			auto variableName = std::string{ startSymbol, size };
			tokens.push_back(CreateToken(TokenType::IDENTIFIER, ValueContainer{ variableName }, currentLine));
		}
		startSymbol = currentSymbol;


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

			ParseDeclaration();
			ParseString();
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
		tokens.emplace_back(TokenType::END, ValueContainer{}, currentLine);
		return true;
	}
}
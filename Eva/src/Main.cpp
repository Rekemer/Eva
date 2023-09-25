#include "iostream"
#include "vector"
#include "map"
#include <stack>
#define DEBUG 1
enum class TokenType : uint8_t {
	// Single-character tokens.
	LEFT_PAREN, RIGHT_PAREN,
	LEFT_BRACE, RIGHT_BRACE,
	COMMA, DOT, MINUS, PLUS,
	SEMICOLON, SLASH, STAR,
	// One or two character tokens.
	BANG, BANG_EQUAL,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	// Literals.
	IDENTIFIER, STRING, NUMBER,
	// Keywords.
	AND, CLASS, ELSE, FALSE,
	FOR, FUN, IF, NIL, OR,
	PRINT, RETURN, SUPER, THIS,
	TRUE, VAR, WHILE,
	ERROR,
	END
};
enum  InCode
{
	CONST_VALUE =1 ,
	ADD =2,
	DIVIDE = 3,
	MULTIPLY= 4,
	SUBSTRACT = 5,
	RETURN = 6,

};
static std::map<TokenType, std::string> tokenStrings = {
		{TokenType::LEFT_PAREN, "LEFT_PAREN"},
		{TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
		{TokenType::LEFT_BRACE, "LEFT_BRACE"},
		{TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
		{TokenType::COMMA, "COMMA"},
		{TokenType::DOT, "DOT"},
		{TokenType::MINUS, "MINUS"},
		{TokenType::PLUS, "PLUS"},
		{TokenType::SEMICOLON, "SEMICOLON"},
		{TokenType::SLASH, "SLASH"},
		{TokenType::STAR, "STAR"},
		{TokenType::BANG, "BANG"},
		{TokenType::BANG_EQUAL, "BANG_EQUAL"},
		{TokenType::EQUAL, "EQUAL"},
		{TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
		{TokenType::GREATER, "GREATER"},
		{TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
		{TokenType::LESS, "LESS"},
		{TokenType::LESS_EQUAL, "LESS_EQUAL"},
		{TokenType::IDENTIFIER, "IDENTIFIER"},
		{TokenType::STRING, "STRING"},
		{TokenType::NUMBER, "NUMBER"},
		{TokenType::AND, "AND"},
		{TokenType::CLASS, "CLASS"},
		{TokenType::ELSE, "ELSE"},
		{TokenType::FALSE, "FALSE"},
		{TokenType::FOR, "FOR"},
		{TokenType::FUN, "FUN"},
		{TokenType::IF, "IF"},
		{TokenType::NIL, "NIL"},
		{TokenType::OR, "OR"},
		{TokenType::PRINT, "PRINT"},
		{TokenType::RETURN, "RETURN"},
		{TokenType::SUPER, "SUPER"},
		{TokenType::THIS, "THIS"},
		{TokenType::TRUE, "TRUE"},
		{TokenType::VAR, "VAR"},
		{TokenType::WHILE, "WHILE"},
		{TokenType::ERROR, "ERROR"},
		{TokenType::END, "END"}
};


struct Token
{
	Token(TokenType type) : type{type}
	{

	}
	TokenType type;
	float value = 0;
};

Token CreateToken(TokenType type, float value)
{
	Token token{type};
	token.value = value;
	return token;
}


struct Expression
{
	float value;
	int childrenCount = 0;
	Expression* left;
	Expression* right;
	TokenType type;
};

std::string tokenToString(TokenType token) {
	

	auto it = tokenStrings.find(token);
	if (it != tokenStrings.end()) {
		return it->second;
	}
	else {
		return "UNKNOWN_TOKEN";
	}
}

Token* currentToken;
const char* startSymbol = nullptr;
const char* currentSymbol = nullptr;
bool panic = false;
std::vector<Token> tokens;
// eat comments too?
void EatWhiteSpace()
{
	while (true)
	{

		switch (*startSymbol) {
		case ' ':
		case '\r':
		case '\t':
			startSymbol++;
			break;
		case '\n':
			startSymbol++;
			break;
		default:
			return;
		}
	}
	currentSymbol = startSymbol;
}

static char PeekNext()
{
	return *(currentSymbol + 1);
}
static char Peek()
{
	return *(currentSymbol);
}

void Eat()
{
	currentSymbol++;
}

void ParseNumber()
{

	// check if number
	auto isDigit = [](char symbol) {return symbol >= '0' && symbol <= '9'; };
	if (isDigit(Peek()))
	{
		// parse a number
		while (isDigit(Peek()))
		{
			Eat();
			if (*currentSymbol == '.' && isDigit(PeekNext()))
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

void ParseOperator()
{
	// check if operator
	if (Peek() == '\0') return;
	switch (Peek())
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
			if (PeekNext() != '\0' && PeekNext() == '=')
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
			if (PeekNext() != '\0' && PeekNext() == '=')
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
			if (PeekNext() != '\0' && PeekNext() == '=')
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
			std::cout << "unexpected character\n";
			tokens.emplace_back(TokenType::ERROR);
			break;
		}	
	}

	if (Peek() != '\0')
	{
		Eat();

	}
}





//expression → equality;
//equality → comparison(("!=" | "==") comparison)*;
//comparison → term((">" | ">=" | "<" | "<=") term)*;
//term → factor(("-" | "+") factor)*;
//factor → unary(("/" | "*") unary)*;
//unary →("!" | "-") unary
//| primary;
//primary → NUMBER | STRING | "true" | "false" | "nil"
//| "(" expression ")";

// either a value or new expression
Expression* ParseExpression();
Expression* UnaryOp();
Expression* Value();
Expression* Factor()
{
	auto left = UnaryOp();
	auto nextToken = (currentToken + 1);
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH;
	if (isMultiplication)
	{

		auto operation = nextToken->type;
		currentToken+=2;
		auto right = UnaryOp();
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
		parent->type = operation;
		return parent;

	}
	return left;
}


Expression* UnaryOp()
{
	bool isUnary = currentToken->type == TokenType::MINUS ? true : false;
	if (isUnary)
	{
		auto prevOp = currentToken->type;
		currentToken++;
		Expression* right = UnaryOp();
		right->type = prevOp;
		return right;
	}

	return Value();
}

Expression* Value()
{
	Expression* node = new Expression();
	node->type = currentToken->type;
	if (currentToken->type == TokenType::NUMBER)
	{
		node->value = currentToken->value;
	}
	if (currentToken->type == TokenType::LEFT_PAREN)
	{
		currentToken += 1;
		node = ParseExpression();
		currentToken += 1;
		if (currentToken->type != TokenType::RIGHT_PAREN)
		{
			std::cout << "error: expected right )\n";
		}
	}
	return node;
	
}


Expression* Term()
{
	auto left = Factor();
	auto nextToken = (currentToken + 1);
	bool isSum = nextToken->type == TokenType::PLUS ||
		nextToken->type == TokenType::MINUS;
	if (isSum)
	{

		auto operation = nextToken->type;
		currentToken += 2;
		auto right = Factor();
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
		parent->type = operation;
		return parent;

	}
	return left;
}

Expression* ParseExpression()
{

	Expression* term = Term();
	return term;
}


std::vector< uint8_t> opCode;
std::vector<float> constants;
std::stack<float> vmStack;

void Execute()
{
	int ipIndex = 0;
	while (true)
	{
		auto inst = opCode[ipIndex];
		switch (inst)
		{
		case InCode::CONST_VALUE:
		{
			vmStack.push(constants[opCode[++ipIndex]]);
			++ipIndex;
			break;
		}
		case InCode::ADD:
		{
			auto v = vmStack.top();
			vmStack.pop();
			auto v2 = vmStack.top();
			vmStack.pop();
			vmStack.push(v+v2);
			++ipIndex;
			break;
		}
		case InCode::SUBSTRACT:
		{
			auto v = vmStack.top();
			vmStack.pop();
			auto v2 = vmStack.top();
			vmStack.pop();
			vmStack.push(v2-v);
			++ipIndex;
			break;
		}
		case InCode::MULTIPLY:
		{
			auto v = vmStack.top();
			vmStack.pop();
			auto v2 = vmStack.top();
			vmStack.pop();
			vmStack.push(v*v2);
			++ipIndex;
			break;
		}
		case InCode::DIVIDE:
		{
			auto v = vmStack.top();
			vmStack.pop();
			auto v2 = vmStack.top();
			vmStack.pop();
			vmStack.push(v2/v);
			++ipIndex;
			break;
		}
		case InCode::RETURN:
		{
			return;
		}
		default:
			break;
		}
	}
}
void Generate(Expression* tree)
{
	if (!tree) return;

	if (tree->type == TokenType::PLUS )
	{
		Generate(tree->left);
		Generate(tree->right);
		opCode.push_back((uint8_t)InCode::ADD);
	}
	else if (tree->type == TokenType::STAR)
	{
		Generate(tree->left);
		Generate(tree->right);
		opCode.push_back((uint8_t)InCode::MULTIPLY);
	}
	else if (tree->type == TokenType::MINUS)
	{
		Generate(tree->left);
		Generate(tree->right);
		opCode.push_back((uint8_t)InCode::SUBSTRACT);
	}
	else if ( tree->type == TokenType::SLASH )
	{
		Generate(tree->left);
		Generate(tree->right);
		opCode.push_back((uint8_t)InCode::DIVIDE);
	}
	else if (tree->type == TokenType::NUMBER)
	{
		opCode.push_back((uint8_t)InCode::CONST_VALUE);
		constants.push_back(tree->value);
		opCode.push_back(constants.size()-1);
	}
	

}

void Print(Expression* tree, int level = 0) {
	if (tree) {
		// Print indentation
		for (int i = 0; i < level; ++i) {
			std::cout << "  "; // Two spaces per level
		}

		// Print the arrow and current node's value and type
		std::cout << "L Value: " << tree->value << " Type: " << tokenToString(tree->type) << std::endl;

		// Recursively print the left subtree with increased indentation
		Print(tree->left, level + 1);

		// Recursively print the right subtree with increased indentation
		Print(tree->right, level + 1);
	}
}
int main(int argc, const char* argv[])
{
	
	std::string input;
	if (argc == 1)
	{
		// repl mode
		while (true)
		{
			std::cout << "> ";
			char line[1024];
			std::cin.getline(line, 1024);
			if (strcmp(line, "exit") == 0)
			{
				break;
			}
			startSymbol = line;
			panic = false;
			currentSymbol = line;
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




		#if DEBUG
			for (auto token : tokens)
			{
				std::cout << tokenToString(token.type) << " ";
			}

			std::cout << "\n";
		#endif // DEBUG
		currentToken = tokens.data();
		Expression* tree = ParseExpression();
		Print(tree);

		Generate(tree);
		opCode.push_back((uint8_t)InCode::RETURN);
		Execute();
		std::cout << "result: " << vmStack.top() << std::endl;
		tokens.clear(); opCode.clear();
		while (vmStack.empty() == false)
		{
			vmStack.pop();
		}
	}
		
	}
	else
	{
		// parse file
	}


	std::cout << "Hello, Eva!\n";
}
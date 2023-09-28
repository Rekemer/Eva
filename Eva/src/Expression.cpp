#include"Expression.h"
#include<iostream>
Expression* Factor(const Token* currentToken)
{
	auto left = UnaryOp(currentToken);
	auto nextToken = (currentToken + 1);
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH;
	if (isMultiplication)
	{

		auto operation = nextToken->type;
		currentToken += 2;
		auto right = Factor(currentToken);
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
		parent->type = operation;
		return parent;

	}
	return left;
}


Expression* UnaryOp(const Token*  currentToken)
{
	bool isUnary = currentToken->type == TokenType::MINUS ? true : false;
	if (isUnary)
	{
		auto prevOp = currentToken->type;
		currentToken++;
		Expression* parent = new Expression();
		Expression* right = UnaryOp(currentToken);
		parent->type = prevOp;
		parent->left = right;
		return parent;
	}

	return Value(currentToken);
}

Expression* Value(const Token* currentToken)
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
		node = ParseExpression(currentToken);
		currentToken += 1;
		if (currentToken->type != TokenType::RIGHT_PAREN)
		{
			std::cout << "error: expected right )\n";
		}
	}
	return node;

}

void Print(Expression* tree, int level) {
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
Expression* Term(const Token* currentToken)
{
	auto left = Factor(currentToken);
	auto nextToken = (currentToken + 1);
	bool isSum = nextToken->type == TokenType::PLUS ||
		nextToken->type == TokenType::MINUS;
	if (isSum)
	{

		auto operation = nextToken->type;
		currentToken += 2;
		auto right = Term(currentToken);
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
		parent->type = operation;
		return parent;

	}
	return left;
}

Expression* ParseExpression(const Token* currentToken)
{

	Expression* term = Term(currentToken);
	return term;
}

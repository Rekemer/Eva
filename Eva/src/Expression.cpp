#include"Expression.h"
#include<iostream>
Expression::Expression(Expression&& e)
{
	childrenCount = e.childrenCount;
	left = e.left;
	right= e.right;
	type =  e.type;
	e.left = e.right = nullptr;
	e.childrenCount = 0;
}
Expression* UnaryOp( Token*& currentToken);
Expression* Value( Token*& currentToken);
Expression* ParseExpression(Token*& currentToken);
 Expression* Factor( Token*& currentToken)
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


 Expression* UnaryOp( Token*&  currentToken)
{
	bool isUnary = currentToken->type == TokenType::MINUS  || currentToken->type == TokenType::BANG? true : false;
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

static Expression* Value( Token*& currentToken)
{
	Expression* node = new Expression();
	node->type = currentToken->type;
	if (currentToken->type == TokenType::NUMBER)
	{
		node->value = std::move(currentToken->value);
	}
	else if (currentToken->type == TokenType::FALSE)
	{
		node->value = std::move(currentToken->value);
	}
	else if  (currentToken->type == TokenType::TRUE)
	{
		node->value = std::move(currentToken->value);
	}
	else if (currentToken->type == TokenType::STRING)
	{
		node->value = std::move(currentToken->value);

	}
	else if (currentToken->type == TokenType::LEFT_PAREN)
	{
		currentToken += 1;
		node = ParseExpression(currentToken);
		currentToken += 1;
		if (currentToken->type != TokenType::RIGHT_PAREN)
		{
			std::cout << "error: expected  )\n";
		}
	}
	return node;

}

void Print(const Expression* tree, int level) {
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
 Expression* Term( Token*& currentToken)
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

 Expression* Comparison( Token*& currentToken)
 {
	 auto left = Term(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::LESS ||
		 nextToken->type == TokenType::LESS_EQUAL || nextToken->type == TokenType::GREATER ||
		 nextToken->type == TokenType::GREATER_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = Comparison(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }


 Expression* Equality( Token*& currentToken)
 {
	 auto left = Comparison(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::EQUAL_EQUAL ||
		 nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = Equality(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

Expression* ParseExpression( Token*& currentToken)
{

	Expression* term = Equality(currentToken);
	return term;
}
bool AST::Build(Token*&  firstToken)
{
	tree = std::make_unique<Expression>(std::move(*ParseExpression(firstToken)) );
	return true;
}


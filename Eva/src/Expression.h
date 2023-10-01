#pragma once
#include "Tokens.h"
#include "Value.h"
#include  <memory>
struct Expression
{
	ValueContainer value;
	int childrenCount = 0;
	Expression* left;
	Expression* right;
	TokenType type;
	Expression() = default;
	Expression(Expression&&);

	Expression(const Expression&) = delete;
	~Expression()
	{
		delete left;
		delete right;

	}
};
//const Token * currentToken = can change a pointer but not the contents
// Token*  const currentToken = can change the contents but not the pointer


void Print(const Expression* tree, int level = 0);
class AST
{
public:
	bool Build(Token*& firstToken);
	const Expression* GetTree()const  { return tree.get(); }
private:
	std::unique_ptr<Expression> tree;
};
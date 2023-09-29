#pragma once
#include "Tokens.h"
#include "Value.h"
struct Expression
{
	ValueContainer value;
	int childrenCount = 0;
	Expression* left;
	Expression* right;
	TokenType type;
};
//const Token * currentToken = can change a pointer but not the contents
// Token*  const currentToken = can change the contents but not the pointer
Expression* ParseExpression( Token*& currentToken);

void Print(Expression* tree, int level = 0); 

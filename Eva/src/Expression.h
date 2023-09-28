#pragma once
#include "Tokens.h"
struct Expression
{
	float value;
	int childrenCount = 0;
	Expression* left;
	Expression* right;
	TokenType type;
};
//const Token * currentToken = can change a pointer but not the contents
// Token*  const currentToken = can change the contents but not the pointer
Expression* ParseExpression(const Token* currentToken);
Expression* UnaryOp(const Token* currentToken);
Expression* Value(const Token* currentToken);
Expression* Factor(const Token* currentToken);
Expression* Term(const Token* currentToken);
Expression* UnaryOp(const Token* currentToken);

void Print(Expression* tree, int level = 0); 

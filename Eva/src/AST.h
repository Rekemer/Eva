#pragma once
#include "Tokens.h"
#include "Value.h"
#include  <memory>
struct Expression
{
	ValueContainer value;
	int childrenCount = 0;
	Expression* left = nullptr;
	Expression* right = nullptr;
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
	void TypeCheck(VirtualMachine& vm);
	const Expression* GetTree()const  { return tree.get(); }
	class VirtualMachine* vm;
private:
	TokenType TypeCheck(Expression* expr, VirtualMachine& vm);
	Expression* UnaryOp(Token*& currentToken);
	Expression* Value(Token*& currentToken);
	Expression* ParseExpression(Token*& currentToken);
	Expression* Factor(Token*& currentToken);
	Expression* Term(Token*& currentToken);
	Expression* Comparison(Token*& currentToken);
	Expression* Equality(Token*& currentToken);
	Expression* LogicalAnd(Token*& currentToken);
	Expression* LogicalOr(Token*& currentToken);
	Expression* Equal(Token*& currentToken);
	Expression* Statement(Token*& currentToken);
	std::unique_ptr<Expression> tree;
};
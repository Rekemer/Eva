#pragma once
#include "Tokens.h"
#include "Value.h"
#include  <memory>
#include  <vector>
struct Expression
{
	ValueContainer value;
	int line =0;
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
struct Scope : public Expression
{
	std::vector<Expression*> expressions;
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
	bool IsPanic() { return m_Panic; }
private:
	TokenType TypeCheck(Expression* expr, VirtualMachine& vm);
	Expression* UnaryOpPrefix(Token*& currentToken);
	Expression* UnaryOpPostfix(Token*& currentToken);
	Expression* Value(Token*& currentToken);
	Expression* ParseExpression(Token*& currentToken);
	Expression* Factor(Token*& currentToken);
	Expression* Term(Token*& currentToken);
	Expression* Comparison(Token*& currentToken);
	Expression* Equality(Token*& currentToken);
	Expression* LogicalAnd(Token*& currentToken);
	Expression* LogicalOr(Token*& currentToken);
	Expression* Equal(Token*& currentToken);
	Expression* EqualOp(Token*& currentToken);
	Expression* Statement(Token*& currentToken);
	std::unique_ptr<Expression> tree;
	bool m_Panic = false;
};
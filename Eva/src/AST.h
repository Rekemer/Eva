#pragma once
#include "Tokens.h"
#include "Value.h"
#include  <memory>
#include  <vector>



struct Node
{
	Node() = default;
	TokenType type;
	int line = 0;
	int childrenCount = 0;
	Node(const Node&);
	
	virtual ~Node()
	{

	};
	template <typename T>
	const T* As() const
	{
		return static_cast<const T*>(this);
	}
};

struct BinaryOp final: public Node
{

};
struct UnaryOp final : public Node
{

};
struct Expression : public Node
{
	ValueContainer value;
	int depth = 0;
	Expression() = default;
	Expression(Expression&&);
	Expression(const Expression&) = delete;

	Node* left = nullptr;
	Node* right = nullptr;
	virtual ~Expression()
	{
		delete left;
		delete right;
	}

};
struct Scope : public Node
{
	std::vector<Node*> expressions;
	
	int popAmount = 0;
};

//const Token * currentToken = can change a pointer but not the contents
// Token*  const currentToken = can change the contents but not the pointer



void Print(const Expression* tree, int level = 0);
class AST
{
public:
	AST(const AST&) = delete;
	AST(AST&& e )
	{
		tree = e.tree;
		e.tree = nullptr;
	}
	AST() = default;
	bool Build(Token*& firstToken);
	void TypeCheck(VirtualMachine& vm);
	const Node* GetTree()const  { return tree; }
	class VirtualMachine* vm;
	bool IsPanic() { return m_Panic; }
	~AST()
	{
		delete tree;
	}
private:
	TokenType TypeCheck(Node* expr, VirtualMachine& vm);
	Node* UnaryOpPrefix(Token*& currentToken);
	Node* UnaryOpPostfix(Token*& currentToken);
	Node* Value(Token*& currentToken);
	Node* ParseExpression(Token*& currentToken);
	Node* Factor(Token*& currentToken);
	Node* Term(Token*& currentToken);
	Node* Comparison(Token*& currentToken);
	Node* Equality(Token*& currentToken);
	Node* LogicalAnd(Token*& currentToken);
	Node* LogicalOr(Token*& currentToken);
	Node* Equal(Token*& currentToken);
	Node* EqualOp(Token*& currentToken);
	Node* Statement(Token*& currentToken);
	Node* EatIf(Token*& currentToken);
	Node* EatBlock(Token*& currentToken);
	void BeginBlock(Token*& currentToken);
	void EndBlock(Token*& currentToken);
private:
	// unique_ptr causes slicing 
	// if std::make_unique<Node>(std::move(expr))
	
	Node* tree;
	bool m_Panic = false;
	inline static int scopeDepth = 0;
	inline static int scopeDeclarations = 0;
};
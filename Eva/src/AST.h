#pragma once
#include "Tokens.h"
#include "Value.h"
#include "Hashtable.h"
#include  <memory>
#include  <stack>
#include  <vector>



struct Node
{
	TokenType type;
	int line = 0;
	int childrenCount = 0;
	Node() = default;
	Node(const Node&) = delete;
	Node(Node&&) = default;
	/*Node::Node(const Node& e)
	{
		childrenCount = e.childrenCount;
		type = e.type;
		line = e.line;

	}*/
	virtual ~Node()
	{

	};
	template <typename T>
	const T* As() const
	{
		return static_cast<const T*>(this);
	}
	template <typename T>
	T* AsMut() const
	{
		return (T*)(this);
	}
};

struct Expression : public Node
{
	ValueContainer value;
	int depth = 0;
	Expression() = default;
	Expression(Expression&&);
	Expression(const Expression&) = delete;

	std::unique_ptr<Node> left = nullptr;
	std::unique_ptr<Node> right = nullptr;

};

;
struct Scope : public Node
{
	std::vector<std::unique_ptr<Node>> expressions;
	HashTable types;
	Scope() = default;
	// Delete the copy constructor and copy assignment operator
	Scope(const Scope&) = delete;
	Scope& operator=(const Scope&) = delete;

	// Default the move constructor and move assignment operator
	Scope(Scope&&) = default;
	Scope& operator=(Scope&&) = default;
	int popAmount = 0;
};
struct For : public Node
{
	Scope initScope;
	std::unique_ptr<Node> init = nullptr;
	std::unique_ptr<Node> condition = nullptr;
	std::unique_ptr<Node> action = nullptr;
	std::unique_ptr<Node> body = nullptr;
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
		tree = std::move(e.tree);
		e.tree = nullptr;
	}
	AST() = default;
	bool Build(Token*& firstToken);
	void TypeCheck(VirtualMachine& vm);
	const Node* GetTree()const  { return tree.get(); }
	class VirtualMachine* vm;
	bool IsPanic() { return m_Panic; }
private:
	void Error(TokenType expectedType, Token*& currentToken, const char* msg);
	TokenType TypeCheck(Node* expr, VirtualMachine& vm);
	std::unique_ptr<Node> UnaryOpPrefix(Token*& currentToken);
	std::unique_ptr<Node> UnaryOpPostfix(Token*& currentToken);
	std::unique_ptr<Node> Value(Token*& currentToken);
	std::unique_ptr<Node> ParseExpression(Token*& currentToken);
	std::unique_ptr<Node> Factor(Token*& currentToken);
	std::unique_ptr<Node> Term(Token*& currentToken);
	std::unique_ptr<Node> Comparison(Token*& currentToken);
	std::unique_ptr<Node> Equality(Token*& currentToken);
	std::unique_ptr<Node> LogicalAnd(Token*& currentToken);
	std::unique_ptr<Node> LogicalOr(Token*& currentToken);
	std::unique_ptr<Node> Equal(Token*& currentToken);
	std::unique_ptr<Node> EqualOp(Token*& currentToken);
	std::unique_ptr<Node> Statement(Token*& currentToken);
	std::unique_ptr<Node> EatIf(Token*& currentToken);
	std::unique_ptr<Node> EatBlock(Token*& currentToken);
	void BeginBlock();
	void EndBlock();
	void Declare(Token*& currentToken, String& str, 
		ValueType declaredType, HashTable& table,
		HashTable& globalTypes,
		Expression* node,
		int offset);
private:
	// unique_ptr causes slicing 
	// if std::make_unique<Node>(std::move(*expr))
	
	std::unique_ptr<Node> tree;
	bool m_Panic = false;
	inline static int scopeDepth = 0;
	// so we now how many pop commands must be executed
	// once block ends
	std::stack<int> scopeDeclarations;
	// to populate types of local variables
	Scope* currentScope;
	std::vector<Scope*> currentScopes;
};
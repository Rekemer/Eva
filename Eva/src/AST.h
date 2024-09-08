#pragma once
#include "Tokens.h"
#include "Value.h"
#include "Hashtable.h"
#include  <memory>
#include  <stack>
#include  <vector>
#include  <sstream>	



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
struct FunctionNode : public Node
{
	Scope paramScope;
	std::unique_ptr<Node> body;
	std::shared_ptr<String> name;
	std::vector<std::unique_ptr<Node>> arguments;
};
struct Call : public Node
{
	std::shared_ptr<String> name;
	std::vector<std::unique_ptr<Node>> args;
};



// Token*  const currentToken = can change the contents but not the pointer
//const Token * currentToken = can change a pointer but not the contents
using Iterator = std::vector<Token>::iterator;

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
	bool Build(Iterator& firstToken);
	void TypeCheck(VirtualMachine& vm);
	const Node* GetTree()const  { return tree.get(); }
	class VirtualMachine* vm;
	bool IsPanic() { return m_Panic; }
	void Fold();
private:
	void CalculateConstant(TokenType op,Expression* left, Expression* right, Expression* newValue);
	Expression*  FoldConstants(Expression* node);
	void FoldBlockConstants(Scope* scope);
	template<typename ...T>
	 void ErrorMult(Token*& currentToken, const char* msg, T... expectedType )
	{
		// fold
		if (((currentToken->type != expectedType) || ...))
		{
			m_Panic = true;
			std::cout << "ERROR[" << (currentToken)->line << "]:" << msg << std::endl;
		}
		else
		{
			currentToken++;
		}
	}

	void TravelSemicolon(Iterator& currentToken);
	void Error(TokenType expectedType, Iterator& currentToken, const char* msg);
	void Error( Iterator& currentToken, const char* msg);
	void Error( Iterator& currentToken, std::stringstream& ss);

	void ErrorTypeCheck(int line, std::stringstream& ss);
	void ErrorTypeCheck(int line, const char* str);

	TokenType TypeCheck(Node* expr, VirtualMachine& vm);
	std::unique_ptr<Node> UnaryOpPrefix(Iterator& currentToken);
	std::unique_ptr<Node> UnaryOpPostfix(Iterator& currentToken);
	std::unique_ptr<Node> Value(Iterator& currentToken);
	std::unique_ptr<Node> ParseExpression(Iterator& currentToken);
	std::unique_ptr<Node> Factor(Iterator& currentToken);
	std::unique_ptr<Node> Term(Iterator& currentToken);
	std::unique_ptr<Node> Comparison(Iterator& currentToken);
	std::unique_ptr<Node> Equality(Iterator& currentToken);
	std::unique_ptr<Node> LogicalAnd(Iterator& currentToken);
	std::unique_ptr<Node> LogicalOr(Iterator& currentToken);
	std::unique_ptr<Node> Declaration(Iterator& currentToken);
	std::unique_ptr<Node> DeclareVariable(Iterator& currentToken);
	std::unique_ptr<Node> DeclareFunction(Iterator& currentToken);
	std::unique_ptr<Node> EqualOp(Iterator& currentToken);
	std::unique_ptr<Node> Assignment(Iterator& currentToken);
	std::unique_ptr<Node> Statement(Iterator& currentToken);
	std::unique_ptr<Node> EatIf(Iterator& currentToken);
	std::unique_ptr<Node> EatBlock(Iterator& currentToken);
	void BeginBlock();
	void EndBlock();

	// offsets go from semicolon
	void DeclareGlobal(Iterator& currentToken,
		ValueType declaredType, HashTable& table,
		HashTable& globalTypes,
		Expression* node,
		int offset);
	void DeclareLocal(Iterator& currentToken,
		VirtualMachine* vm, Expression* node,ValueType type,int offset);

	void BindValue(Iterator& currentToken, Node* variable);
private:
	// unique_ptr causes slicing 
	// if std::make_unique<Node>(std::move(*expr))
	
	std::unique_ptr<Node> tree;
	bool m_Panic = false;
	// we can have loop in the loop, 
	// so we need to support to notice breaks and continues
	std::stack<bool> m_ParseLoops;
	inline static int scopeDepth = 0;
	// so we now how many pop commands must be executed
	// once block ends
	std::stack<int> scopeDeclarations;
	// to populate types of local variables
	Scope* currentScope = nullptr;
	std::vector<Scope*> currentScopes;

};
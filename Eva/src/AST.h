#pragma once
#include "Tokens.h"
#include "Value.h"
#include "Hashtable.h"
#include "Local.h"
#include  <memory>
#include  <stack>
#include  <vector>
#include  <sstream>	






// Token*  const currentToken = can change the contents but not the pointer
//const Token * currentToken = can change a pointer but not the contents
using Iterator = std::vector<Token>::iterator;
struct Node;
struct Expression;
void Print(const Expression* tree, int level = 0);
class AST
{
public:
	AST(const AST&) = delete;
	AST(AST&& e )
	{
		tree = std::move(e.tree);
		stackSim = e.stackSim;
		e.tree = nullptr;
	}
	AST() = default;
	StackSim Build(Iterator& firstToken);
	void TypeCheck(VirtualMachine& vm);
	const Node* GetTree()const  { return tree.get(); }
	class VirtualMachine* vm;
	bool IsPanic() { return m_Panic; }
	void Fold();
private:

	

	void PartialFold(Node* leftOperandSibling, Node* rightOperandSibling,
		bool isLeftLBase, bool isRightLBase, Node* baseLeft, Node* baseRight,
		Expression* baseExpression, Expression* accumulateNode, bool isRightDirection);
	void CalculateConstant(TokenType op,Expression* left, Expression* right, Expression* newValue);
	Node*  FoldConstants(Node* node);
	void FoldBlockConstants(Scope* scope);
	void StartFolding(Node* node);
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

	void AddLocal(String& name, int currentScope)
	{
		auto endIterator = stackSim.locals.begin() + stackSim.m_StackPtr;
		auto iter = std::find_if(stackSim.locals.begin(), endIterator, [&](auto& local)
			{
				return local.name == name && local.depth == currentScope;
			});

		if (iter != endIterator)
		{
			assert(false && "variable already declared in current scope");
		}
		stackSim.locals[stackSim.m_StackPtr].name = name;
		stackSim.locals[stackSim.m_StackPtr++].depth = currentScope;
	}
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
public:
	StackSim stackSim;
};
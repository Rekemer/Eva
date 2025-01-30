#pragma once
#include "Tokens.h"
#include "Value.h"
#include "Hashtable.h"
#include "Local.h"
#include "Nodes.h"
#include  <memory>
#include  <stack>
#include  <vector>
#include  <sstream>	



namespace Eva
{


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
		AST(AST&& e) = default;
		//{
		//	tree = std::move(e.tree);
		//	e.tree = nullptr;
		//}
		AST() = default;
		void Build(Iterator& firstToken);
		class Compiler* compiler;
		void TypeCheck();
		const Node* GetTree()const { return tree.get(); }
		bool IsPanic() { return m_Panic; }
		void Fold();
	private:

		// Helper functions for different node types
		TokenType TypeCheckEqual(Node* expr, TokenType to, TokenType from);

		TokenType TypeCheckIfStatement(Node* node);

		TokenType TypeCheckFunctionCall(Node* node);

		TokenType TypeCheckReturnStatement(Node* node);

		TokenType TypeCheckFunctionDefinition(Node* node);

		TokenType TypeCheckBlock(Node* node);

		TokenType TypeCheckForLoop(Node* node);

		TokenType TypeCheckExpression(Node* node);

		TokenType TypeCheckBinaryOperation(Expression* expr, TokenType leftType, TokenType rightType);

		TokenType TypeCheckUnaryOperation(Expression* expr, TokenType operandType);

		TokenType TypeCheckIdentifier(Expression* expr);

		TokenType TypeCheckVariableDeclaration(Expression* expr, TokenType leftType, TokenType rightType);


		void UpdateScope(int stackOffset, Scope* prevScope, Scope* newScope);
		void PartialFold(Node* leftOperandSibling, Node* rightOperandSibling,
			bool isLeftLBase, bool isRightLBase, Node* baseLeft, Node* baseRight,
			Expression* baseExpression, Expression* accumulateNode, bool isRightDirection);
		void CalculateConstant(TokenType op, Expression* left, Expression* right, Expression* newValue);
		Node* FoldConstants(Node* node);
		void FoldBlockConstants(Scope* scope);
		void StartFolding(Node* node);
		template<typename ...T>
		void ErrorMult(Token*& currentToken, const char* msg, T... expectedType)
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
		void Error(Iterator& currentToken, const char* msg);
		void Error(Iterator& currentToken, std::stringstream& ss);

		void ErrorTypeCheck(int line, const std::string& str);
		TokenType TypeCheck(Node* expr);
		std::unique_ptr<Node> CallFunc(std::string_view str, Iterator& currentToken, bool isGlobal);
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
		void DeclareLocal(Iterator& currentToken, Expression* node, ValueType type, int offset);

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
		int indexCurrentScope = -1;
	};
}
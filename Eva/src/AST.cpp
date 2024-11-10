#include "AST.h"
#include <iostream>
#include <cassert>
#include <format>
#include <algorithm>
#include "VirtualMachine.h"
#include "Value.h"
#include "Tokens.h"
#include "TokenConversion.h"


Expression::Expression(Expression&& e) : Node(std::move(e))
{
	left = std::move(e.left);
	right = std::move(e.right);
	value = std::move(e.value);
	depth = e.depth;
	e.left.reset();
	e.right.reset();
	e.childrenCount = 0;
}
 std::unique_ptr<Node> AST::Factor( Iterator& currentToken)
{
	auto left = UnaryOpPrefix(currentToken);
	// Handle consecutive multiplication/division/modulus operations
	while (true)  
	{
		auto nextToken = (currentToken);  

		bool isMultiplication = nextToken->type == TokenType::STAR ||
			nextToken->type == TokenType::SLASH ||
			nextToken->type == TokenType::PERCENT;

		if (!isMultiplication)
		{
			break;  
		}

		auto operation = currentToken->type;
		auto parent = std::make_unique<Expression>();
		parent->line = currentToken->line;
		currentToken++;  

		auto right = UnaryOpPrefix(currentToken);

		parent->left = std::move(left);
		parent->right = std::move(right);
		parent->type = operation;

		left = std::move(parent);
	}

	return left;
}



 void AST::BindValue(Iterator& currentToken, Node* variable)
 {
	 auto expr = static_cast<Expression*>(variable);
	 expr->right = LogicalOr(currentToken);
	 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
 }

 
 void AST::DeclareGlobal(Iterator& currentToken,
	  ValueType type, HashTable& table,
	 HashTable& globalTypes, Expression* node,
	 int offset)
 {
	 auto str = currentToken->value.AsString();
	 globalTypes.Add(str, type);
	 // define global variable
	 // to note global that variable is declared, so that in value it can be used
	 auto variableName = table.Add(str, ValueContainer{})->key;
	 // it will initialize node with the name of a variable
	 node->left = LogicalOr(currentToken);

	 node->depth = 0;
	 currentToken += offset;
 }
 void AST::DeclareLocal(Iterator& currentToken,
	 VirtualMachine* vm, Expression* node,ValueType type, int offset)
 {
	 auto str = currentToken->value.AsString();
	 scopeDeclarations.top()++;
	 currentScope->AddLocal(str, scopeDepth);
	 node->left = LogicalOr(currentToken);

	 currentScope->types.Add(str, type);
	 currentToken += offset;
 }
 std::unique_ptr<Node> AST::UnaryOpPrefix( Iterator&  currentToken)
{
	bool isUnary = currentToken->type == TokenType::MINUS  || currentToken->type == TokenType::BANG? true : false;
	if (isUnary)
	{
		auto prevOp = currentToken->type;
		auto parent = std::make_unique<Expression>();
		parent->line = currentToken->line;
		currentToken++;
		parent->type = prevOp;
		parent->left = UnaryOpPrefix(currentToken);
		//parent->value.type = LiteralToType(parent->left->type);
		return parent;
	}
	auto value = Value(currentToken);
	auto postfix = UnaryOpPostfix(currentToken);
	if (postfix)
	{
		auto expr = static_cast<Expression*>(postfix.get());
		expr->left = std::move(value);
		//if (currentToken->type == TokenType::SEMICOLON)
		//{
		//	currentToken++;
		//}
		return postfix;
	}
	
	return value;
}
 std::unique_ptr<Node> AST::UnaryOpPostfix(Iterator& currentToken)
 {
	 bool isDouble = currentToken->type == TokenType::MINUS_MINUS || currentToken->type == TokenType::PLUS_PLUS;
	 if (isDouble)
	 {
		 auto prevOp = currentToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->type = prevOp;
		 currentToken++;
		 // child is initialized by the caller
		 return parent;
	 }
	 return nullptr;
 }

 std::unique_ptr<Node> AST::Value( Iterator& currentToken)
{
	auto node = std::make_unique<Expression>();
	auto identiferToken = currentToken;
	node->line = currentToken->line;
	node->type = currentToken->type;
	if (currentToken->type == TokenType::INT_LITERAL)
	{
		node->value = std::move(currentToken->value);
	}
	else if (currentToken->type == TokenType::FLOAT_LITERAL)
	{
		node->value = std::move(currentToken->value);
	}
	else if (currentToken->type == TokenType::FALSE)
	{
		node->value = std::move(currentToken->value);
	}
	else if  (currentToken->type == TokenType::TRUE)
	{
		node->value = std::move(currentToken->value);
	}
	else if (currentToken->type == TokenType::STRING_LITERAL)
	{
		node->value = std::move(currentToken->value);
	}
	// only creates node, it must be already declared
	else if (currentToken->type == TokenType::IDENTIFIER)
	{	
		auto str = currentToken->value.AsString();
		auto& globalTable = vm->GetGlobals();
		auto entry = globalTable.Get(str);
		auto isGlobal = entry->IsInit();

		if ((currentToken + 1)->type == TokenType::LEFT_PAREN)
		{
			if (!isGlobal)
			{
				std::stringstream ss;
				ss << "There is no function " << str << " to call";
				Error(currentToken,ss);
				return{};
			}

			// call a function
			auto call = std::make_unique<Call>();
			call->name = str;
			call->type = TokenType::LEFT_PAREN;
			currentToken+=2;
			while (currentToken->type != TokenType::RIGHT_PAREN)
			{
				auto arg = LogicalOr(currentToken);
				call->args.push_back(std::move(arg));
				if (currentToken->type != TokenType::RIGHT_PAREN)
				{
					Error(TokenType::COMMA, currentToken, "Arguments must be separated with comma");
				}
			}

			Error(TokenType::RIGHT_PAREN, currentToken, "Arguments list must end with ) ");
			//if (currentToken->type == TokenType::SEMICOLON)
			//Error(TokenType::SEMICOLON, currentToken, "Function call must end with ;");
			return call;
		}
		else
		{
			// if local scope then read global or local
			if (scopeDepth > 0)
			{
				auto [isLocalDeclared, index, scopeDepthChecked] = currentScope->IsLocalExist(str,scopeDepth);
				
				if (isLocalDeclared)
				{
					// should check whether it is declared variable

					auto variableName = currentToken->value.AsString();
					node->value = ValueContainer(variableName);
					node->depth = scopeDepthChecked;
				}
				else if (isGlobal)
				{
					auto variableName = entry->key;
					node->value = ValueContainer(variableName);
					node->depth = 0;
				}
				else
				{
					m_Panic = true;
					std::cout << "ERROR[" << (currentToken)->line << "]: " <<
						"The name " << str << " is used but not declared " << std::endl;
				}
			}
			// if scope == 0 then can read only global
			else if (isGlobal && scopeDepth == 0)
			{
				auto variableName = entry->key;
				node->value = ValueContainer(variableName);
				node->depth = 0;
			}
			else
			{
				m_Panic = true;
				std::cout << "ERROR[" << (currentToken)->line << "]: " <<
					"The name " << str << " is used but not declared " << std::endl;
			}

		}
	}
	else if (currentToken->type == TokenType::LEFT_PAREN)
	{
			currentToken += 1;
			auto node = ParseExpression(currentToken);
			Error(TokenType::RIGHT_PAREN,currentToken,"Expected )");
			return node;
	}
	currentToken += 1;
	return node;

}

void Print(const Expression* tree, int level) {
	if (tree) {
		// Print indentation
		for (int i = 0; i < level; ++i) {
			std::cout << "  "; // Two spaces per level
		}

		// Print the arrow and current node's value and type
		std::cout << "L Value: " << tree->value << " Type: " << tokenToString(tree->type) << std::endl;

		// Recursively print the left subtree with increased indentation
		//Print(tree->left, level + 1);

		// Recursively print the right subtree with increased indentation
		//Print(tree->right, level + 1);
	}
}
 std::unique_ptr<Node> AST::Term( Iterator& currentToken)
{
	auto left = Factor(currentToken);
	auto nextToken = (currentToken );
	bool isSum = nextToken->type == TokenType::PLUS ||
		nextToken->type == TokenType::MINUS;
	if (isSum)
	{

		auto operation = nextToken->type;
		auto parent = std::make_unique<Expression>();
		parent->line = currentToken->line;
		auto isMinus = nextToken->type == TokenType::MINUS;
		// to fix  2 -1 -1 = 2 
		if (!isMinus)
		{
			currentToken++;
		}
		auto right = Term(currentToken);
		parent->left = std::move(left);
		parent->right= std::move(right);
		parent->type = TokenType::PLUS;
		return parent;

	}
	return left;
}

 std::unique_ptr<Node> AST::Comparison(Iterator& currentToken) {
	 auto left = Term(currentToken);  

	 while (true) {
		 auto nextToken = currentToken;
		 bool isCompare = nextToken->type == TokenType::LESS ||
			 nextToken->type == TokenType::LESS_EQUAL ||
			 nextToken->type == TokenType::GREATER ||
			 nextToken->type == TokenType::GREATER_EQUAL ||
			 nextToken->type == TokenType::BANG_EQUAL;

		 if (!isCompare) break;  

		 auto operation = nextToken->type;
		 currentToken++;  

		 auto right = Term(currentToken);  

		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;

		 left = std::move(parent);  
	 }

	 return left;
 }
 std::unique_ptr<Node> AST::LogicalAnd(Iterator& currentToken) {
	 auto left = Equality(currentToken);  

	 while (true) {
		 auto nextToken = currentToken;
		 bool isAnd = nextToken->type == TokenType::AND;

		 if (!isAnd) break;  

		 auto operation = nextToken->type;
		 currentToken++;  

		 auto right = Equality(currentToken); 

		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;

		 left = std::move(parent);  
	 }

	 return left;
 }
 std::unique_ptr<Node> AST::LogicalOr(Iterator& currentToken) {
	 auto left = LogicalAnd(currentToken);  

	 while (true) {
		 auto nextToken = currentToken;
		 bool isOr = nextToken->type == TokenType::OR;

		 if (!isOr) break;  

		 auto operation = nextToken->type;
		 currentToken++;  

		 auto right = LogicalAnd(currentToken);  

		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;

		 left = std::move(parent);  
	 }

	 return left;
 }
 std::unique_ptr<Node> AST::Equality(Iterator& currentToken) {
	 auto left = Comparison(currentToken);

	 while (true) {
		 auto nextToken = currentToken;
		 bool isCompare = nextToken->type == TokenType::EQUAL_EQUAL ||
			 nextToken->type == TokenType::BANG_EQUAL;

		 if (!isCompare) break;

		 auto operation = nextToken->type;
		 currentToken += 1;  

		 auto right = Comparison(currentToken);  

		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;

		 left = std::move(parent);  
	 }

	 return left;
 }



 std::unique_ptr<Node> AST::EqualOp(Iterator& currentToken)
 {
	 auto isOp = (currentToken + 1)->type == TokenType::PLUS_EQUAL
		 || (currentToken + 1)->type == TokenType::STAR_EQUAL
		 || (currentToken + 1)->type == TokenType::SLASH_EQUAL
		 || (currentToken + 1)->type == TokenType::MINUS_EQUAL;
	 if (isOp)
	 {
		 auto operation = (currentToken+1)->type;
		 
		 auto parent = std::make_unique<Expression>();
		 parent->type = operation;
		 parent->line = currentToken->line;
		 parent->left = LogicalOr(currentToken);
		 currentToken++;
		 parent->right = LogicalOr(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 //currentToken++;
		 return parent;
	 }

	 

	 return LogicalOr(currentToken);

 }
 // fun foo() : int {}
 std::unique_ptr<Node> AST::DeclareFunction(Iterator& currentToken)
 {
	 currentToken++;
	 auto name = currentToken->value.AsString();
	 currentToken++;
	 Error(TokenType::LEFT_PAREN,currentToken,"Function argument list must start with (");
	 auto function = std::make_unique<FunctionNode>();
	 auto& globalVariables = vm->GetGlobals();
	 auto& globalTypes = vm->GetGlobalsType();
	 auto funcValue = globalVariables.Add(name, LiteralToType(TokenType::FUN))
		 ->value.AsFunc();
	 function->name = name;
	 function->type = TokenType::FUN;

	 auto prevScope = currentScope;
	 BeginBlock();
	 currentScope = &function->paramScope;
	 currentScope->depth = scopeDepth;
	 currentScope->prevScope = prevScope;

	 while (currentToken->type != TokenType::RIGHT_PAREN)
	 {
		 auto arg = Declaration(currentToken);
		 auto name = currentScope->stack.LastLocal();
		 auto declaredType = currentScope->types.Get(name)->value.type;
		 funcValue->argTypes.push_back(declaredType);
		 
		 function->arguments.push_back(std::move(arg));
		 if (currentToken->type != TokenType::RIGHT_PAREN)
		 {
			Error(TokenType::COMMA,currentToken,"Arguments must be separated with comma");
		 }
	 }
	 Error(TokenType::RIGHT_PAREN,currentToken,"Function argument list must end with )");
	 if (currentToken->type != TokenType::COLON)
	 {
		// void function
		globalTypes.Add(function->name, ValueType::NIL);
	 }
	 else
	 {
		Error(TokenType::COLON,currentToken,"Function must declare return type");
		globalTypes.Add(function->name, LiteralToType(currentToken->type));
		currentToken++;
	 }
	 function->body = EatBlock(currentToken);
	 auto body = function->body->AsMut<Scope>();
	 
	 for (auto& arg : function->arguments)
	 {
		 auto expr = arg->As<Expression>()->left->As<Expression>();
		 auto name = expr->value.AsString();
		 auto entry = function->paramScope.types.Get(name);
		 body->types.Add(name, entry->value.type);
	 }
	 EndBlock();
	 return function;
 }
 std::unique_ptr<Node> AST::DeclareVariable(Iterator& currentToken)
 {
	 auto& table = vm->GetGlobals();
	 auto& globalsType = vm->GetGlobalsType(); 

	 auto str =currentToken->value.AsString();
	 auto declaredType = (currentToken + 2)->type;
	 auto isEqualSign = (currentToken + 3)->type == TokenType::EQUAL;
	 auto entry = table.Get(str);
	 auto [isType, type] = IsVariableType(declaredType);
	 auto node = std::make_unique<Expression>();
	 node->type = TokenType::DECLARE;
	 node->depth = scopeDepth;
	 if (!entry->IsInit() && scopeDepth == 0)
	 {
		 node->line = currentToken->line;

		 // if there are tokens that correspond to declaration
		 // number :int = 2; number :=2;
		 if (isType)
		 {
			DeclareGlobal(currentToken, LiteralToType(type), table,
			 globalsType, node.get(), 2);
			 if (isEqualSign)
			 {
				currentToken++;
				BindValue(currentToken,node.get());
			 }
		 }
		 else
		 {
			 auto isEqualSign = (currentToken + 2)->type == TokenType::EQUAL;
			 if (isEqualSign)
			 {
				 DeclareGlobal(currentToken, ValueType::DEDUCE, table, globalsType, node.get(), 2);
				 BindValue(currentToken,node.get());
			 }
		 }
		 return node;
	 }
	 else
	 {
		 // define a local variable
		 if (scopeDepth > 0)
		 {
			 if (isType)
			 {
				 DeclareLocal(currentToken, vm, node.get(), LiteralToType(type), 2);
				 if (isEqualSign)
				 {
					 currentToken++;
					 BindValue(currentToken, node.get());
				 }
			 }
			 else
			 {
				 DeclareLocal(currentToken, vm, node.get(), ValueType::DEDUCE, 2);
				 BindValue(currentToken,node.get());
			 }
			 return node;
		 }
	 }
 }

 bool isLiteral(TokenType type)
 {
	 switch (type)
	 {
	 case TokenType::INT_LITERAL:
	 case TokenType::FLOAT_LITERAL:
	 case TokenType::STRING_LITERAL:
	 case TokenType::TRUE:
	 case TokenType::FALSE:
	 return true;
	 default:
	 return false;
	 }
 }

 void AST::CalculateConstant(TokenType op,Expression* left, Expression* right, Expression* newValue)
 {
	 auto node = newValue;
	 switch (op)
	 {
	 case TokenType::PLUS:
		 node->value = ValueContainer::Add(left->value, right->value);
		 break;
	 case TokenType::MINUS:
		 node->value = ValueContainer::Substract(left->value, right->value);
		 break;
	 case TokenType::STAR:
		 node->value = ValueContainer::Multiply(left->value, right->value);
		 break;
	 case TokenType::SLASH:
		 node->value = ValueContainer::Divide(left->value, right->value);
		 break;
	 case TokenType::EQUAL_EQUAL:
		 node->value = ValueContainer::Equal(left->value, right->value);
		 break;
	 case TokenType::BANG_EQUAL:
	 {
		 auto newValue = ValueContainer::Equal(left->value, right->value);
		 node->value = !newValue.AsRef<bool>();
		 break;
	 }
	 case TokenType::GREATER:
		 node->value = ValueContainer::Greater(left->value, right->value);
		 break;
	 case TokenType::GREATER_EQUAL:
	 {
		 auto newValue = ValueContainer::Equal(left->value, right->value);
		 node->value = !newValue.AsRef<bool>();
		 break;
	 }
	 case TokenType::LESS:
		 node->value = ValueContainer::Less(left->value, right->value);
		 break;
	 case TokenType::LESS_EQUAL:
	 {
		 auto newValue = ValueContainer::Greater(left->value, right->value);
		 node->value = !newValue.AsRef<bool>();
		 break;
	 }
	 default:
		 assert(false && "unknown binary operation on literals");
		 break;
	 }
 }

 void AST::PartialFold(Node* leftOperandSibling, Node* rightOperandSibling,
	 bool isLeftLBase, bool isRightLBase, Node* baseLeft, Node* baseRight,
	 Expression* baseExpression, Expression* accumulateNode, bool isRightDirection)
 {
	 auto isLeftLiteralRightChild = isLiteral(leftOperandSibling->type);
	 auto isRightLiteralRightChild = isLiteral(rightOperandSibling->type);
	 TokenType rightType = TokenType::LEFT_PAREN;

	 auto leftNode =  FoldConstants(leftOperandSibling);
	 auto rightNode = FoldConstants(rightOperandSibling);

	 if (rightNode->type == TokenType::MINUS || leftNode->type == TokenType::MINUS) {
		 return;
	 }

	 // if partial folding and the subtree has a node which operates on two variables
	 auto areBothVariables = leftNode->type == TokenType::IDENTIFIER && rightNode->type == TokenType::IDENTIFIER;
	 if (areBothVariables) return;

	 if (isLeftLBase && isLeftLiteralRightChild) {
		 CalculateConstant(baseExpression->type, accumulateNode, static_cast<Expression*>(leftNode), accumulateNode);
		 rightType = leftNode->type;
		 baseExpression->right = std::move(isRightDirection ? static_cast<Expression*>(baseRight)->right
			 : static_cast<Expression*>(baseRight)->right);
	 }
	 else if (isLeftLBase && isRightLiteralRightChild) {
		 CalculateConstant(baseExpression->type, accumulateNode, static_cast<Expression*>(rightNode), accumulateNode);
		 rightType = rightNode->type;
		 baseExpression->right = std::move(isRightDirection ? static_cast<Expression*>(baseLeft)->left
			 : static_cast<Expression*>(baseRight)->left);
	 }

	 assert(rightType != TokenType::LEFT_PAREN);

	 auto newType = (accumulateNode->type == TokenType::FLOAT_LITERAL || rightType == TokenType::FLOAT_LITERAL)
		 ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;

	 accumulateNode->type = newType;
 }



 

 
 Node* AST::FoldConstants(Node* node)
 {
	 if (!node)
	 {
		 return {};
	 }

	 if (node->type == TokenType::LEFT_PAREN)
	 {
		 auto call = static_cast<Call*>(node);
		 for (auto& arg : call->args)
		 {
			 auto node = FoldConstants(arg.get());
			 if (node != arg.get())
			 {
				 arg =  std::unique_ptr<Node>(node);
			 }
		 }
		 return call;
	 }
	 auto expr = static_cast<Expression*>(node);
	 if (isLiteral(expr->type))
	 {
		 return expr;
	 }
	 if (expr->type == TokenType::MINUS && expr->left->type != TokenType::IDENTIFIER && !expr->right)
	 {
		auto left = expr->left->AsMut<Expression>();

		Node* node = new Expression();
		auto exprInverted= static_cast<Expression*>(node);

		 if (isLiteral(expr->left->type))
		 {
			// could we perhaps do it without allocation?
			exprInverted->type = left->type;
			exprInverted->value = left->value;
			exprInverted->value.Negate();
			return exprInverted;
		 }
		// in case -(1-2)
		node = static_cast<Node*>(FoldConstants(static_cast<Expression*>(expr->left.get())));
		exprInverted->value.Negate();
		return node;
	 }
	 // check if both constant
		// replace subtree with the new node
	 if (expr->type == TokenType::IDENTIFIER )
	 {
		 return expr;
	 }
	 // should have unary and binary minus to remove this check
	 auto isUnaryMinus = expr->type == TokenType::MINUS ? expr->right != nullptr : true;
	 auto isBinaryOp = IsBinaryOp(expr->type) && isUnaryMinus;
	 if (isBinaryOp)
	 {
		 // recurse so we can have series of constant + constant + ...
		 auto left = FoldConstants(static_cast<Expression*>(expr->left.get()));
		 auto leftExpr = static_cast<Expression*>(left);
		//if (IsBinaryOp(expr->left->type))
		//{
		//	 expr->left = std::unique_ptr<Node>(left);
		//}
		 auto right = FoldConstants(static_cast<Expression*>(expr->right.get()));
		 auto rightExpr = static_cast<Expression*>(right);
		// if (IsBinaryOp(expr->right->type))
		// {
		//
		//	 expr->right = std::unique_ptr<Node>(right);
		// }
		 auto isLitL = isLiteral(left->type);
		 auto isLitR = isLiteral(right->type);\
		
		 // case constant op constant 
		 if (isLitL && isLitR)
		 {
			 auto isBool = left->type == TokenType::TRUE || left->type == TokenType::FALSE &&
				 right->type == TokenType::TRUE || right->type == TokenType::FALSE;
			 auto node = new Expression();
			 if (isBool)
			 {
				 switch (expr->type)
				 {
				 case TokenType::AND:
					 node->value = ValueContainer::And(leftExpr->value, rightExpr->value);
					 break;
				 case TokenType::OR:
					 node->value = ValueContainer::Or(leftExpr->value, rightExpr->value);
					 break;
				 case TokenType::EQUAL_EQUAL:
					 node->value = ValueContainer::Equal(leftExpr->value, rightExpr->value);
					 break;
				 case TokenType::BANG_EQUAL:
				 {
					 auto newValue = ValueContainer::Equal(leftExpr->value, rightExpr->value);
					 node->value = !newValue.AsRef<bool>();
					 break;
				 }

				 default:
					 assert(false && "unknown binary operation on bool literals");
					 break;
				 }
				 node->type = node->value.As<bool>() ? TokenType::TRUE : TokenType::FALSE;
			 }
			 else
			 {
				 CalculateConstant(expr->type,leftExpr,rightExpr,node);
				 if (IsBinaryBoolOp(expr->type))
				 {
					 node->type = node->value.As<bool>() ? TokenType::TRUE : TokenType::FALSE;
				 }
				 else
				 {
					 auto newType = (left->type == TokenType::FLOAT_LITERAL || right->type == TokenType::FLOAT_LITERAL)
						 ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;
					 node->type = newType;
				 }
			 }
			 return node;
		 }

		 // it can be partial folding like 3 + a + 5;
		 // we need to check children of operation to see if we can fold
		 // accumulate on left node of current operation
		 else if ( isLitL && IsBinaryOp(rightExpr->type) && rightExpr->type == expr->type)
		 {
			 PartialFold(rightExpr->left.get(), rightExpr->right.get(), isLitL, isLitR,left,right,expr,leftExpr,false);
			 right = nullptr;
		 }
		 // accumulate on right node of current operation
		 else if (isLitR && IsBinaryOp(leftExpr->type) && leftExpr->type == expr->type)
		 {
			PartialFold(leftExpr->left.get(), leftExpr->right.get(), isLitL, isLitR, left, right,expr, rightExpr,true);
			 left = nullptr;
			
		 }
		


		 if (expr->right.get() != right && right != nullptr)
		 {
			expr->right = std::unique_ptr<Node>(right);
		 }
		 if (expr->left.get() != left && left != nullptr)
		 {
			expr->left = std::unique_ptr<Node>(left);
		 }
		
	 }
	return expr;
 }


 void AST::StartFolding(Node* node)
 {
	 auto updateNode = [](std::unique_ptr<Node>& target, Node* newNode) {
		 if (newNode && target.get() != newNode) {
			 target = std::unique_ptr<Node>(newNode);
		 }
	 };
	if (node->type == TokenType::FOR)
	{
		 auto forNode = node->AsMut<For>();
		 StartFolding(forNode->init.get());
		 auto newNodeCond= FoldConstants(forNode->condition.get());
		 updateNode(forNode->condition, newNodeCond);
		 auto newNodeAction= FoldConstants(forNode->action.get());
		 updateNode(forNode->action, newNodeAction);
	}
	else
	 {
		 auto expr = node->AsMut<Expression>();
		 auto left = expr->left.get()->AsMut<Expression>();
		 auto right = expr->right.get()->AsMut<Expression>();

		 auto newNodeLeft = FoldConstants(left);
		 updateNode(expr->left, newNodeLeft);
		 auto newNodeRight = FoldConstants(right);
		 updateNode(expr->right, newNodeRight);
	 }
 }

 void AST::FoldBlockConstants(Scope* block)
 {
	 for (auto& node : block->expressions)
	 {
		 if (node->type == TokenType::BLOCK)
		 {
			 FoldBlockConstants(static_cast<Scope*>(node.get()));
			 return;
		 }
		 StartFolding(node.get());
		 
	 }
 }

 void AST::Fold()
 {
	 
	 if (tree->type == TokenType::BLOCK)
	 {
		 auto block = static_cast<Scope*>(tree.get());
		 FoldBlockConstants(block);
	 }
	 else if (tree->type == TokenType::FOR)
	 {
		 auto forNode = static_cast<For*>(tree.get());
		 StartFolding(forNode->init.get());
		 StartFolding(forNode->condition.get());
		 StartFolding(forNode->action.get());
	 }
	 else if (tree->type == TokenType::FUN)
	 {
		 auto funNode = static_cast<FunctionNode*>(tree.get());
		 FoldBlockConstants(static_cast<Scope*>(funNode->body.get()));
	 }
	 else
	 {
		 StartFolding(tree.get());
	 }
 }

 std::unique_ptr<Node> AST::Declaration(Iterator& currentToken)
 {
	 bool isVariable = currentToken->type == TokenType::IDENTIFIER;
	 bool isFunc = currentToken->type == TokenType::FUN;
	 bool isDeclaration = (currentToken+1)->type == TokenType::COLON;
	 bool isAssignment = (currentToken+1)->type == TokenType::EQUAL;
	 auto varToken = currentToken;
	 Token* eqToken;
	 
	 auto isDeclarared = [&](auto name)
	 {
		auto isGlobalDeclared = vm->GetGlobals().IsExist(name);
		
		bool isLocalDeclared = false;
		if (currentScope != nullptr)
		{
			 isLocalDeclared = currentScope->types.IsExist(name);
		}
		if ((isLocalDeclared  || (isGlobalDeclared && scopeDepth == 0)))
		{
			std::stringstream ss;
			ss << "The name " << name.data() << " is already declared ";
			Error(currentToken, ss);
			return true;
		}
		 return false;
	 };
	 
	 if (isVariable && isDeclaration)
	 {	
		 auto name = currentToken->value.AsString();
		 auto check = isDeclarared(name);
		 if (check) return {};
		 return DeclareVariable(currentToken);
	 }
	 else if (isFunc)
	 {	
		 auto name = (currentToken+1)->value.AsString();
		 auto check = isDeclarared(name);
		 if (check) return {};
		 return DeclareFunction(currentToken);
	 }
	 return Assignment(currentToken);
	 //return EqualOp(currentToken);
 }

 std::unique_ptr<Node> AST::Assignment(Iterator& currentToken)
 {
	 auto isOp = (currentToken + 1)->type == TokenType::EQUAL;
	 if (isOp)
	 {
		 auto operation = (currentToken + 1)->type;

		 auto parent = std::make_unique<Expression>();
		 parent->type = operation;
		 parent->line = currentToken->line;
		 parent->left = LogicalOr(currentToken);
		 currentToken++;
		 parent->right = LogicalOr(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 return parent;
	 }
	 return EqualOp(currentToken);

 }

 std::unique_ptr<Node> AST::EatBlock(Iterator& currentToken)
 {
	 Error(TokenType::LEFT_BRACE, currentToken, "Expected { at the beginning of the block");
	 
	 auto block = std::make_unique<Scope>();
	 
	 int offset = 0;
	 if (currentScope)
	 {
		 offset = currentScope->stack.m_StackPtr;
	 }
	 auto prevScope = currentScope;
	 UpdateScope(offset, prevScope, block.get());
	 //currentScope = block.get();
	 //currentScope->depth = scopeDepth;
	 //currentScope->stack.m_StackPtr = offset;
	 //if (currentScope != prevScope) currentScope->prevScope = prevScope;
	 block->type = TokenType::BLOCK;
	 while (currentToken->type != TokenType::RIGHT_BRACE &&
		 currentToken->type != TokenType::END)
	 {
		 auto expression = Statement(currentToken);
		 block->expressions.push_back(std::move(expression));
	 }
	 Error(TokenType::RIGHT_BRACE, currentToken, "Expected } at the end of the block");
	 block->popAmount = scopeDeclarations.size() > 0 ? scopeDeclarations.top() : 0;
	 currentScope = prevScope;
	 return block;
 }

 // eats then branch and if conition
 std::unique_ptr<Node> AST::EatIf(Iterator& currentToken)
 {
	 auto ifnode = std::make_unique<Expression>();
	 ifnode->line = currentToken->line;
	 ifnode->type = TokenType::IF;
	 currentToken++;
	 ifnode->left = LogicalOr(currentToken);
	 //currentToken += 1;
	 ifnode->right = std::make_unique<Expression>();
	 BeginBlock();
	 auto then = EatBlock(currentToken);
	 EndBlock();
	 //ifnode->type= solutionType 
	 // get all then statements in range { } 

	 auto expr = static_cast<Expression*>(ifnode->right.get());
	 expr->right = std::move(then);
	 return ifnode;
 }
 void AST::TravelSemicolon(Iterator& currentToken)
 {
	 while (currentToken->type != TokenType::SEMICOLON)
	 {
		 currentToken++;
	 }
	 currentToken++;
 }
 void AST::ErrorTypeCheck(int line,const std::string& str)
 {
	 m_Panic = true;
	 std::cout << "ERROR[" << line << "] " << str << std::endl;
 }
 void AST::Error(Iterator& currentToken, std::stringstream& ss)
 {
	 m_Panic = true;
	 std::cout << "ERROR[" << (currentToken)->line << "]:" << ss.str() << std::endl;
	 TravelSemicolon(currentToken);
 }

 void AST::Error(Iterator& currentToken, const char* msg)
 {
	 m_Panic = true;
	 std::cout << "ERROR[" << (currentToken)->line << "]:" << msg << std::endl;
	 TravelSemicolon(currentToken);
 }
 void AST::Error(TokenType expectedType, Iterator& currentToken, const char* msg)
 {
	 if (currentToken->type != expectedType)
	 {
		 m_Panic = true;
		 Error(currentToken, msg);
	 }
	 else
	 {
		 currentToken++;
	 }
 }
	

 // each statement is
 // required to have zero stack effect
 std::unique_ptr<Node> AST::Statement(Iterator& currentToken)
 {


	 if (currentToken->type == TokenType::PRINT)
	 {
		 auto printNode = std::make_unique<Expression>();
		 printNode->line = currentToken->line;
		 printNode->type = TokenType::PRINT;
		 currentToken += 1;
		 printNode->left = LogicalOr(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 return printNode;
	 }
	 // ifnode represents the whole if statement
	 // left child is condition
	 // right child is the then and else branches
	 // right right is then
	 // right left  is else
	 

	// [ifnode]
	//	 /     \
// [condition] [flows]
		//	   /    \
		//	 [else][then]

	 else if (currentToken->type == TokenType::IF)
	 {
		 auto ifnode = EatIf(currentToken);
		 auto expr = static_cast<Expression*>(ifnode.get());
		 auto flows = static_cast<Expression*>(expr->right.get());
		 // check elif blocks
		 while (currentToken->type == TokenType::ELIF)
		 {
			 auto elifnode = EatIf(currentToken);
			 flows->left = std::move(elifnode);
			 flows = flows->left->AsMut<Expression>()->
				 right->AsMut<Expression>();

		 }
		 
		 // else block
		 if (currentToken->type == TokenType::ELSE)
		 {
			 currentToken++;
			 BeginBlock();
			 auto elseScope = EatBlock(currentToken);
			 EndBlock();
			 flows->left = std::move(elseScope);
		 }
		 return ifnode;
	 }
	 else if (currentToken->type == TokenType::WHILE)
	 {
		 m_ParseLoops.push(true);
		 auto whileNode = std::make_unique<Expression>();
		 whileNode->type = TokenType::WHILE;
		 whileNode->line = currentToken->line;
		 currentToken++;
		 whileNode->left = LogicalOr(currentToken);
		 BeginBlock();
		 whileNode->right = EatBlock(currentToken);
		 EndBlock();
		 m_ParseLoops.pop();
		 return whileNode;
	 }
	 else if (currentToken->type == TokenType::FOR)
	 {
		 m_ParseLoops.push(true);
		 // can be classic c for without ()
		 // can be for 1..10
		 auto forNode = std::make_unique<For>();
		 forNode->type = TokenType::FOR;
		 forNode->line = currentToken->line;
		 auto isDoubleDot = (currentToken + 5)->type == TokenType::DOUBLE_DOT;
		 auto isIdentifier= (currentToken + 1)->type == TokenType::IDENTIFIER;
		 // can be a..b  1..b b..2  1..2
		 if (isDoubleDot)
		 {
			 currentToken++;

			 int offset = 0;
			 if (currentScope)
			 {
				 offset = currentScope->stack.m_StackPtr;
			 }
			 auto prevScope = currentScope;
			 BeginBlock();
			 UpdateScope(offset, currentScope,&forNode->initScope);




			 auto begin= (currentToken+3);
			 auto end= (currentToken+5);
			 auto startValue = begin->value;
			 if (isIdentifier)
			 {
				 std::vector<Token> forLoop =
				 {
					 *(currentToken),
					 CreateToken(TokenType::COLON,{},currentToken->line),
					 CreateToken(TokenType::EQUAL,{},currentToken->line),
					 CreateToken(begin->type,std::move(startValue),currentToken->line),
					 CreateToken(TokenType::SEMICOLON,{},currentToken->line),
					*(currentToken),
					 CreateToken(TokenType::LESS,{},currentToken->line),
					 *end,
					*(currentToken),
					 CreateToken(TokenType::PLUS_PLUS,{},currentToken->line),
					 CreateToken(TokenType::END,{},currentToken->line),
				 };
				 auto forLoopPtr = forLoop.begin();
				 forNode->init = Declaration(forLoopPtr);
				 assert(forLoopPtr->type == currentToken->type);
				 forNode->condition =LogicalOr(forLoopPtr);
				 assert(forLoopPtr->type == currentToken->type);
				 forNode->action = Statement(forLoopPtr);
				 assert(forLoopPtr->type == TokenType::END);
				 currentToken+=6;
				 BeginBlock();
				 forNode->body = EatBlock(currentToken);
				 EndBlock();
 				 forNode->initScope.popAmount = scopeDeclarations.size() > 0 ? scopeDeclarations.top() : 0;
				 currentScope = prevScope;
				 EndBlock();
			 }
			 else
			 {
				 Error(currentToken,"missing Iterator& declaration");
			 }

		 }
		 else if (isIdentifier)
		 {
			 currentToken += 1;
			 auto prevScope = currentScope;
			 int offset = 0;
			 if (currentScope)
			 {
				 offset = currentScope->stack.m_StackPtr;
			 }
			 // so we can create i Iterator&
			 BeginBlock();
			 UpdateScope(offset, currentScope, &forNode->initScope);
			 // init node
			 forNode->init = Declaration(currentToken);
			 
			 // check and action node
			 forNode->condition = LogicalOr(currentToken);
			 //currentToken += 1;
			 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");

			 forNode->action= Statement(currentToken);
			 BeginBlock();
			 forNode->body = EatBlock(currentToken);
			 EndBlock();
			 forNode->initScope.popAmount = scopeDeclarations.size() > 0 ?
				 scopeDeclarations.top() : 0;
			 currentScope = prevScope;
			 EndBlock();
		 }
		 m_ParseLoops.pop();
		 return forNode;
	 }
	 else if (currentToken->type == TokenType::CONTINUE ||
		 currentToken->type == TokenType::BREAK)
	 {
		 if (m_ParseLoops.size() > 0 && m_ParseLoops.top() == true)
		 {
			 auto loopAction = std::make_unique<Node>();
			 loopAction->line = currentToken->line;
			 loopAction->type= currentToken->type;
			 currentToken++;
			 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
			 return loopAction;
		 }
		 else
		 {
			 Error(currentToken,"Continue and break can be used only in the loop");
		 }
	 }
	 else if (currentToken->type == TokenType::RETURN)
	 {
		 auto returnNode  = std::make_unique<Expression>();
		 returnNode->line = currentToken->line;
		 returnNode->type = currentToken->type;
		 currentToken++;
		 returnNode->left = Assignment(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 return returnNode;
	 }
	 else if (currentToken->type == TokenType::LEFT_BRACE)
	 {
		 BeginBlock();
		 auto scope = EatBlock(currentToken);
		 EndBlock();
		 return scope;
	 }
	 else
	 {
		auto node = Declaration(currentToken);
		if (currentToken->type == TokenType::SEMICOLON)
		{
			currentToken++;
		}
		return node;
	 }
 }
 
 void AST::UpdateScope(int stackOffset, Scope* prevScope, Scope* newScope)
 {
	 currentScope = newScope;
	 currentScope->depth = scopeDepth;
	 if (newScope != prevScope) currentScope->prevScope = prevScope;
	 currentScope->stack.m_StackPtr = stackOffset;
 }
 void AST::BeginBlock()
 {
	 scopeDepth++;
	 scopeDeclarations.push(0);
 }
 void AST::EndBlock()
 {
	 scopeDepth--;
	 scopeDeclarations.pop();
 }
std::unique_ptr<Node> AST::ParseExpression( Iterator& currentToken)
{

	auto tree = Statement(currentToken);
	return tree;
}
void AST::Build(Iterator& firstToken)
{
	tree = ParseExpression(firstToken);
}

void AST::TypeCheck(VirtualMachine& vm)
{
	TypeCheck(tree.get(), vm);
}

TokenType AST::TypeCheck(Node* node, VirtualMachine& vm)
{
	if (!node) {
		// Handle null node if necessary
		return TokenType::END;
	}

	switch (node->type)
	{
	case TokenType::IF:
		return TypeCheckIfStatement(node, vm);

	case TokenType::LEFT_PAREN:
		return TypeCheckFunctionCall(node, vm);

	case TokenType::RETURN:
		return TypeCheckReturnStatement(node, vm);

	case TokenType::FUN:
		return TypeCheckFunctionDefinition(node, vm);

	case TokenType::CONTINUE:
	case TokenType::BREAK:
		return node->type;

	case TokenType::BLOCK:
		return TypeCheckBlock(node, vm);

	case TokenType::FOR:
		return TypeCheckForLoop(node, vm);

	default:
		return TypeCheckExpression(node, vm);
	}
}

// Helper functions for different node types

TokenType AST::TypeCheckIfStatement(Node* node, VirtualMachine& vm)
{
	auto expr = static_cast<Expression*>(node);
	// Check condition
	TypeCheck(expr->left.get(), vm);

	// Check 'then' branch
	auto thenExpr = expr->right->As<Expression>()->right.get();
	TypeCheck(thenExpr, vm);

	// Check 'else' branch if it exists
	auto elseExpr = expr->right->As<Expression>()->left.get();
	if (elseExpr)
	{
		TypeCheck(elseExpr, vm);
	}

	return TokenType::END;
}

TokenType AST::TypeCheckFunctionCall(Node* node, VirtualMachine& vm)
{
	auto call = static_cast<Call*>(node);
	auto& globals = vm.GetGlobals();
	auto funcEntry = globals.Get(call->name);
	if (!funcEntry)
	{
		ErrorTypeCheck(call->line, "Undefined function '" + call->name + "'");
		return TokenType::NIL;
	}

	auto funcValue = funcEntry->value.AsFunc();
	if (call->args.size() != funcValue->argTypes.size())
	{
		ErrorTypeCheck(call->line, "Incorrect number of arguments for function '" + call->name + "'");
		return TokenType::NIL;
	}

	for (size_t i = 0; i < call->args.size(); ++i)
	{
		auto& arg = call->args[i];
		auto argType = LiteralToType(TypeCheck(arg.get(), vm));
		auto paramType = funcValue->argTypes[i];

		if (!IsCastable(paramType, argType))
		{
			std::stringstream ss;
			ss << "Argument " << i + 1 << " of function '" << call->name << "' expects type "
				<< ValueToStr(paramType) << ", but got " << ValueToStr(argType);
			ErrorTypeCheck(call->line, ss.str());
			return TokenType::NIL;
		}
	}

	// Return the function's return type
	auto funcTypeEntry = vm.GetGlobalsType().Get(call->name);
	if (!funcTypeEntry || !funcTypeEntry->IsInit())
	{
		ErrorTypeCheck(call->line, "Function type information missing for '" + call->name + "'");
		return TokenType::NIL;
	}

	return TypeToLiteral(funcTypeEntry->value.type);
}

TokenType AST::TypeCheckReturnStatement(Node* node, VirtualMachine& vm)
{
	auto expr = static_cast<Expression*>(node);
	auto retType = TypeCheck(expr->left.get(), vm);

	if (retType == TokenType::NIL)
	{
		ErrorTypeCheck(node->line, "Cannot return NIL value");
	}

	return retType;
}

TokenType AST::TypeCheckFunctionDefinition(Node* node, VirtualMachine& vm)
{
	auto funcNode = static_cast<FunctionNode*>(node);
	auto funcTypeEntry = vm.GetGlobalsType().Get(funcNode->name);

	if (!funcTypeEntry || !funcTypeEntry->IsInit())
	{
		ErrorTypeCheck(funcNode->line, "Function '" + funcNode->name + "' is not properly declared");
		return TokenType::NIL;
	}

	auto actualReturnType = TypeCheck(funcNode->body.get(), vm);
	auto declaredReturnType = funcTypeEntry->value.type;

	if (declaredReturnType != ValueType::NIL)
	{
		if (actualReturnType == TokenType::BLOCK)
		{
			ErrorTypeCheck(funcNode->line, "Could not determine the actual return type of the function");
			return TokenType::NIL;
		}

		if (TypeToLiteral(declaredReturnType) != actualReturnType)
		{
			std::stringstream ss;
			ss << "Function '" << funcNode->name << "' declared return type "
				<< ValueToStr(declaredReturnType) << ", but returns " << tokenToString(actualReturnType);
			ErrorTypeCheck(funcNode->line, ss.str());
			return TokenType::NIL;
		}

		return TypeToLiteral(declaredReturnType);
	}

	return TokenType::NIL;
}

TokenType AST::TypeCheckBlock(Node* node, VirtualMachine& vm)
{
	auto block = static_cast<Scope*>(node);

	currentScopes.push_back(block);
	BeginBlock();
	block->depth = scopeDepth;

	TokenType returnType = TokenType::BLOCK;

	for (auto& expr : block->expressions)
	{
		auto type = TypeCheck(expr.get(), vm);
		if (expr->type == TokenType::RETURN)
		{
			returnType = type;
		}
	}

	EndBlock();
	currentScopes.pop_back();

	return returnType;
}

TokenType AST::TypeCheckForLoop(Node* node, VirtualMachine& vm)
{
	auto forNode = static_cast<For*>(node);

	currentScopes.push_back(&forNode->initScope);
	BeginBlock();

	TypeCheck(forNode->init.get(), vm);
	TypeCheck(forNode->condition.get(), vm);
	TypeCheck(forNode->action.get(), vm);
	TypeCheck(forNode->body.get(), vm);

	EndBlock();
	currentScopes.pop_back();

	return TokenType::FOR;
}
TokenType AST::TypeCheckEqual(Node* expr, TokenType to, TokenType from)
{
	if (!IsCastable(LiteralToType(to), LiteralToType(from)))
	{
		ErrorTypeCheck(expr->line, std::format("Cannot cast from {} to {}", tokenToString(to), tokenToString(from)));
		return TokenType::NIL;
	}
	return to;
}
TokenType AST::TypeCheckExpression(Node* node, VirtualMachine& vm)
{
	auto expr = static_cast<Expression*>(node);
	TokenType leftType = TokenType::END;
	TokenType rightType = TokenType::END;

	if (expr->left)
	{
		leftType = TypeCheck(expr->left.get(), vm);
	}

	if (expr->right)
	{
		rightType = TypeCheck(expr->right.get(), vm);
	}

	switch (expr->type)
	{
	case TokenType::PERCENT:
		expr->value.type = ValueType::INT;
		return TokenType::INT_LITERAL;

	case TokenType::PLUS:
	case TokenType::MINUS:
	case TokenType::STAR:
	case TokenType::SLASH:
		if (expr->right == nullptr)
		{
			return TypeCheckUnaryOperation(expr, leftType);
		}
		else return TypeCheckBinaryOperation(expr, leftType, rightType);

	case TokenType::EQUAL_EQUAL:
	case TokenType::BANG_EQUAL:
	case TokenType::AND:
	case TokenType::OR:
	case TokenType::LESS:
	case TokenType::LESS_EQUAL:
	case TokenType::GREATER:
	case TokenType::GREATER_EQUAL:
	{
		expr->value.type = ValueType::BOOL;
		return TokenType::BOOL_TYPE;
	}

	case TokenType::IDENTIFIER:
		return TypeCheckIdentifier(expr, vm);

	case TokenType::DECLARE:
		return TypeCheckVariableDeclaration(expr, leftType, rightType, vm);

	case TokenType::BANG:
	case TokenType::MINUS_MINUS:
	case TokenType::PLUS_PLUS:
		return TypeCheckUnaryOperation(expr, leftType);
	case TokenType::EQUAL:
		return TypeCheckEqual(expr, leftType,rightType);
	default:
		return expr->type;
	}
}


TokenType AST::TypeCheckBinaryOperation(Expression* expr, TokenType leftType, TokenType rightType) {
	// Check for NIL types
	if (leftType == TokenType::NIL || rightType == TokenType::NIL) {
		ErrorTypeCheck(expr->line, "NIL operand in binary operation");
		return TokenType::NIL;
	}

	// Check if both operands are integers
	if (leftType == TokenType::INT_LITERAL && rightType == TokenType::INT_LITERAL) {
		expr->value.type = ValueType::INT;
		return TokenType::INT_LITERAL;
	}

	// Check if both operands are floats
	if (leftType == TokenType::FLOAT_LITERAL && rightType == TokenType::FLOAT_LITERAL) {
		expr->value.type = ValueType::FLOAT;
		return TokenType::FLOAT_LITERAL;
	}

	// Check if one operand is an integer and the other is a float
	if ((leftType == TokenType::INT_LITERAL && rightType == TokenType::FLOAT_LITERAL) ||
		(leftType == TokenType::FLOAT_LITERAL && rightType == TokenType::INT_LITERAL)) {
		expr->value.type = ValueType::FLOAT; // Promote to float
		return TokenType::FLOAT_LITERAL;
	}

	// Check if both operands are strings
	if (leftType == TokenType::STRING_LITERAL && rightType == TokenType::STRING_LITERAL) {
		expr->value.type = ValueType::STRING;
		return TokenType::STRING_LITERAL;
	}

	// If one operand is a string, return an error unless both are strings
	if (leftType == TokenType::STRING_LITERAL || rightType == TokenType::STRING_LITERAL) {
		ErrorTypeCheck(expr->line, "Invalid operation between STRING and non-STRING types");
		return TokenType::NIL;
	}

	ErrorTypeCheck(expr->line, "Incompatible operand types for binary operation");
	return TokenType::NIL;
}

TokenType AST::TypeCheckUnaryOperation(Expression* expr, TokenType operandType)
{
	if (operandType == TokenType::END)
	{
		ErrorTypeCheck(expr->line, "Unary operator missing operand");
		return TokenType::NIL;
	}

	expr->value.type = LiteralToType(operandType);
	return operandType;
}

TokenType AST::TypeCheckIdentifier(Expression* expr, VirtualMachine& vm)
{
	Entry* entry = nullptr;

	if (expr->depth > 0)
	{
		// Look up in current scopes
		auto name = expr->value.AsString();
		for (auto it = currentScopes.rbegin(); it != currentScopes.rend(); ++it) {
			auto scope = *it;
			if (scope->types.IsExist(name))
			{
				entry = scope->types.Get(name);
				break;
			}
		}

		if (!entry)
		{
			ErrorTypeCheck(expr->line, "Undefined variable '" + name + "'");
			return TokenType::NIL;
		}
	}
	else
	{
		// Look up in global scope
		auto name = expr->value.AsString();
		entry = vm.GetGlobalsType().Get(name);
		if (!entry || !entry->IsInit())
		{
			ErrorTypeCheck(expr->line, "Undefined global variable '" + name + "'");
			return TokenType::NIL;
		}
	}

	return TypeToLiteral(entry->value.type);
}

TokenType AST::TypeCheckVariableDeclaration(Expression* expr, TokenType leftType, TokenType rightType, VirtualMachine& vm)
{
	if (leftType != TokenType::DEDUCE && !IsCastable(LiteralToType(leftType), LiteralToType(rightType)))
	{
		ErrorTypeCheck(expr->line, std::format("Cannot cast from {} to {}", tokenToString(rightType),tokenToString(leftType) ));
		return TokenType::NIL;
	}
	if (rightType == TokenType::NIL)
	{
		ErrorTypeCheck(expr->line, "Cannot assign NIL value to variable");
		return TokenType::NIL;
	}

	if (leftType == TokenType::DEDUCE)
	{
		// Type inference
		auto varName = expr->left->As<Expression>()->value.AsString();
		ValueType inferredType = LiteralToType(rightType);

		if (expr->depth > 0)
		{
			// Update type in current scope

			for (auto it = currentScopes.rbegin(); it != currentScopes.rend(); ++it) {
				auto scope = *it;
				if (scope->types.IsExist(varName))
				{
					auto entry = scope->types.Get(varName);
					entry->value.UpdateType(inferredType);
					break;
				}
			}
		}
		else
		{
			// Update type in global scope
			auto entry = vm.GetGlobalsType().Get(varName);
			if (entry)
			{
				entry->value.UpdateType(inferredType);
			}
		}
	}

	return rightType;
}

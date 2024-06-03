#include"AST.h"
#include<iostream>
#include<cassert>
#include "VirtualMachine.h"


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
 std::unique_ptr<Node> AST::Factor( Token*& currentToken)
{
	auto left = UnaryOpPrefix(currentToken);
	auto nextToken = (currentToken );
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH;
	if (isMultiplication)
	{
		//currentToken++;
		auto operation = currentToken->type;
		auto parent = std::make_unique<Expression>();
		parent->line = currentToken->line;
		currentToken++;
		auto right = Factor(currentToken);
		parent->left = std::move(left);
		parent->right = std::move(right);
		parent->type = operation;
		return parent;

	}
	return left;
}


 ValueType LiteralToType(TokenType literalType)
 {
	 switch (literalType)
	 {
	 case TokenType::INT_LITERAL:
	 case TokenType::INT_TYPE:
		 return ValueType::INT;
		 break;
	 case TokenType::FLOAT_LITERAL:
	 case TokenType::FLOAT_TYPE:
		 return ValueType::FLOAT;
		 break;
	 case TokenType::STRING_LITERAL:
	 case TokenType::STRING_TYPE:
		 return ValueType::STRING;
		 break;
	 case TokenType::FALSE:
	 case TokenType::TRUE:
	 case TokenType::BOOL_TYPE:
		 return ValueType::BOOL;
		 break;
		 // += case: identifer has left child as + and right chlid as = which is value to add to the variable 
	//case TokenType::PLUS:
	//	 return ValueType::NIL;
	 default:
		 assert(false);
		 break;
	 }
 }
 TokenType TypeToLiteral(ValueType valueType)
 {
	 switch (valueType)
	 {
	 case ValueType::INT:
		 return TokenType::INT_LITERAL;
		 break;
	 case ValueType::FLOAT:
		 return TokenType::FLOAT_LITERAL;
		 break;
	 case ValueType::STRING:
		 return TokenType::STRING_LITERAL;
		 break;
	 case ValueType::BOOL:
		 return TokenType::BOOL_TYPE;
		 break;
	 case ValueType::DEDUCE:
		 return TokenType::DEDUCE;
		 break;
	 default:
		 assert(false);
		 break;
	 }
 }
 void AST::Declare(Token*& currentToken,
	 String& str, ValueType type, HashTable& table,
	 HashTable& globalTypes, Expression* node,
	 int offset)
 {
	 globalTypes.Add(str.GetStringView(), type);
	 // define global variable
	 table.Add(str.GetStringView(), type);
	 // to note global that variable is declared, so that in value it can be used
	 auto variableName = table.Add(str.GetStringView(), ValueContainer{})->key;
	 // it will initialize node with the name of a variable
	 node->left = LogicalOr(currentToken);
	 currentToken += offset;
	 auto leftExpression = static_cast<Expression*>(node->left.get());
	 leftExpression->value.type = type;
	 //node->left->value = ValueContainer((Object*)variableName);
	 node->depth = scopeDepth;
	 node->right = LogicalOr(currentToken);
	 //currentToken++;
 }
 std::unique_ptr<Node> AST::UnaryOpPrefix( Token*&  currentToken)
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
		currentToken++;
		return postfix;
	}
	
	return value;
}
 std::unique_ptr<Node> AST::UnaryOpPostfix(Token*& currentToken)
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

 std::unique_ptr<Node> AST::Value( Token*& currentToken)
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
	// only creates node, prior to that in equal declared
	else if (currentToken->type == TokenType::IDENTIFIER)
	{	
		auto& str = currentToken->value.As<String&>();
		auto& globalTable = vm->GetGlobals();
		auto entry = globalTable.Get(str.GetStringView());
		auto isGlobal = entry->key != nullptr;
		// if local scope then read global or local
		if (scopeDepth > 0)
		{	

			auto [isLocalDeclared, index] = vm->IsLocalExist(str);
			if (isLocalDeclared)
			{
				// should check whether it is declared variable
				auto& variableName = currentToken->value.As<String&>();
				node->value = ValueContainer((Object*)&variableName);
				node->depth = scopeDepth;
			}
			else if (isGlobal)
			{
				auto variableName = entry->key;
				node->value = ValueContainer((Object*)variableName);
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
			node->value = ValueContainer((Object*)variableName);
		}
		else
		{
			m_Panic = true;
			std::cout << "ERROR[" << (currentToken)->line << "]: " <<
				"The name " << str << " is used but not declared " << std::endl;
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
 std::unique_ptr<Node> AST::Term( Token*& currentToken)
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

		currentToken++;
		auto right = Term(currentToken);
		parent->left = std::move(left);
		parent->right = std::move(right);
		parent->type = operation;
		return parent;

	}
	return left;
}

 std::unique_ptr<Node> AST::Comparison( Token*& currentToken)
 {
	 auto left = Term(currentToken);
	 auto nextToken = (currentToken );
	 bool isCompare = nextToken->type == TokenType::LESS ||
		 nextToken->type == TokenType::LESS_EQUAL || nextToken->type == TokenType::GREATER ||
		 nextToken->type == TokenType::GREATER_EQUAL || nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken ++;
		 auto right = Comparison(currentToken);
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 std::unique_ptr<Node> AST::Equality(Token*& currentToken)
 {
	 auto left = Comparison(currentToken);
	 auto nextToken = (currentToken );
	 bool isCompare = nextToken->type == TokenType::EQUAL_EQUAL ||
		 nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 1;
		 auto right = Equality(currentToken);
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 std::unique_ptr<Node> AST::LogicalAnd(Token*& currentToken)
 {
	 auto left = Equality(currentToken);
	 auto nextToken = (currentToken );
	 bool isAnd = nextToken->type == TokenType::AND;
	 if (isAnd)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 1;
		 auto right = LogicalAnd(currentToken);
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 std::unique_ptr<Node> AST::EqualOp(Token*& currentToken)
 {
	 auto isOp = (currentToken+1)->type == TokenType::PLUS_EQUAL
		 || (currentToken + 1)->type == TokenType::STAR_EQUAL
		 || (currentToken + 1)->type == TokenType::SLASH_EQUAL
		 || (currentToken + 1)->type == TokenType::MINUS_EQUAL;
	 if (isOp)
	 {
		 auto operation = (currentToken+1)->type;
		 
		 auto parent = std::make_unique<Expression>();
		 parent->type = operation;
		 parent->left = LogicalOr(currentToken);
		 currentToken ++;
		 parent->right = LogicalOr(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 //currentToken++;
		 return parent;
	 }
	 return Declaration(currentToken);

 }
 
 std::unique_ptr<Node> AST::Declaration(Token*& currentToken)
 {
	 bool isVariable = currentToken->type == TokenType::IDENTIFIER;
	 bool isDeclaration = (currentToken+1)->type == TokenType::COLON;
	 bool isAssignment = (currentToken+1)->type == TokenType::EQUAL;
	 auto varToken = currentToken;
	 Token* eqToken;

	 

	 if (isDeclaration)
	 {
		auto& table = vm->GetGlobals();
		auto& globalsType = vm->GetGlobalsType();
		auto& str= currentToken->value.As<String&>();

		auto declaredType = (currentToken + 2)->type;
		auto isEqualSign = (currentToken + 3)->type == TokenType::EQUAL;
		auto entry = table.Get(str.GetStringView());
		auto [isType,type]= IsVariableType(declaredType);
		auto node = std::make_unique<Expression>();
		node->type = TokenType::EQUAL;
		node->depth = scopeDepth;
		if (entry->key == nullptr && scopeDepth == 0)
		{
			node->line = currentToken->line;

			// if there are tokens that correspond to declaration
			// number :int = 2; number :=2;
			if (isType)
			{
				if (isEqualSign)
				{
					Declare(currentToken, str, LiteralToType(type),table,
						globalsType,node.get(),3);
				}

			}
			else
			{
				auto isEqualSign = (currentToken + 2)->type == TokenType::EQUAL;
				if (isEqualSign)
				{
					Declare(currentToken, str, ValueType::DEDUCE, table, globalsType, node.get(), 2);
				}
			}
			//commented so we can run expression tests
			Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");

			return node;
		}
		else
		{
			// define a local variable
			if (scopeDepth > 0)
			{
				if (isType)
				{
					if (isEqualSign)
					{
						scopeDeclarations.top()++;
						vm->AddLocal(str, scopeDepth);
						node->left = LogicalOr(currentToken);
						auto leftExpression = static_cast<Expression*>(node->left.get());

						currentScope->types.Add(str.GetStringView(), LiteralToType(declaredType));
						leftExpression->value.type = LiteralToType(type);
						currentToken += 3;
						node->right = LogicalOr(currentToken);
						Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
						return node;
					}
				}
				else
				{
					scopeDeclarations.top()++;
					vm->AddLocal(str, scopeDepth);
					node->left = LogicalOr(currentToken);
					auto leftExpression = static_cast<Expression*>(node->left.get());

					currentScope->types.Add(str.GetStringView(), ValueType::DEDUCE);
					leftExpression->value.type = ValueType::DEDUCE;
					currentToken += 2;
					node->right = LogicalOr(currentToken);
					Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
					return node;
				}
				
			}
		}
		 
	 }
	 else if (isAssignment)
	 {
		 auto node = std::make_unique<Expression>();
		 node->line = (currentToken+1)->line;
		 node->type = (currentToken+1)->type;
		 node->type = TokenType::EQUAL;
		 node->left = Value(currentToken);
		 currentToken++;
		 node->right = LogicalOr(currentToken);
		 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
		 return node;
	 }
	 return LogicalOr(currentToken);
 }

 std::unique_ptr<Node> AST::LogicalOr(Token*& currentToken)
 {
	 auto left = LogicalAnd(currentToken);
	 auto nextToken = (currentToken );
	 bool isOr = nextToken->type == TokenType::OR;
	 if (isOr)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 1;
		 auto right = LogicalOr(currentToken);
		 parent->left = std::move(left);
		 parent->right = std::move(right);
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 std::unique_ptr<Node> AST::EatBlock(Token*& currentToken)
 {
	 Error(TokenType::LEFT_BRACE, currentToken, "Expected { at the beginning of the block");
	 BeginBlock();
	 auto block = std::make_unique<Scope>();
	 currentScope = block.get();
	 block->type = TokenType::BLOCK;
	 while (currentToken->type != TokenType::RIGHT_BRACE &&
		 currentToken->type != TokenType::END)
	 {
		 auto expression = Statement(currentToken);
		 block->expressions.push_back(std::move(expression));
	 }
	 Error(TokenType::RIGHT_BRACE, currentToken, "Expected } at the end of the block");
	 block->popAmount = scopeDeclarations.size() > 0 ? scopeDeclarations.top() : 0;
	 EndBlock();
	 return block;
 }

 // eats then branch and if conition
 std::unique_ptr<Node> AST::EatIf(Token*& currentToken)
 {
	 auto ifnode = std::make_unique<Expression>();
	 ifnode->line = currentToken->line;
	 ifnode->type = TokenType::IF;
	 currentToken++;
	 ifnode->left = LogicalOr(currentToken);
	 //currentToken += 1;
	 ifnode->right = std::make_unique<Expression>();
	 auto then = EatBlock(currentToken);
	 //ifnode->type= solutionType 
	 // get all then statements in range { } 

	 auto expr = static_cast<Expression*>(ifnode->right.get());
	 expr->right = std::move(then);
	 return ifnode;
 }

 void AST::Error(TokenType expectedType, Token*& currentToken, const char* msg)
 {
	 if (currentToken->type != expectedType)
	 {
		 m_Panic = true;
		 std::cout << "ERROR[" << (currentToken)->line << "]:" << msg << std::endl;
	 }
	 else
	 {
		 currentToken++;
	 }
 }

 // each statement is
 // required to have zero stack effect
 std::unique_ptr<Node> AST::Statement(Token*& currentToken)
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
	 // left child is condition
	 // right child is the then and else branches
	 // right right is then
	 // right left  is else
	 else if (currentToken->type == TokenType::IF)
	 {
		 auto ifnode = EatIf(currentToken);
		 auto tmpNode = ifnode.get();
		 auto expr = static_cast<Expression*>(tmpNode);
		 auto right = static_cast<Expression*>(expr->right.get());
		 // check elif blocks
		 while (currentToken->type == TokenType::ELIF)
		 {
			 auto elifnode = EatIf(currentToken);
			 right->left = std::move(elifnode);
			 tmpNode = right->left.get();

		 }
		 
		 // else block
		 if (currentToken->type == TokenType::ELSE)
		 {
			 currentToken++;
			 auto elseScope = EatBlock(currentToken);
			 right->left = std::move(elseScope);
		 }
		 return ifnode;
	 }
	 else if (currentToken->type == TokenType::WHILE)
	 {
		 auto whileNode = std::make_unique<Expression>();
		 whileNode->type = TokenType::WHILE;
		 whileNode->line = currentToken->line;
		 currentToken++;
		 whileNode->left = LogicalOr(currentToken);
		 whileNode->right = EatBlock(currentToken);
		 return whileNode;
	 }
	 else if (currentToken->type == TokenType::FOR)
	 {
		 // can be classic c for without ()
		 // can be for 1..10
		 auto forNode = std::make_unique<For>();
		 forNode->type = TokenType::FOR;
		 forNode->line = currentToken->line;
		 auto isDoubleDot = (currentToken + 2)->type == TokenType::DOUBLE_DOT;
		 auto isIdentifier= (currentToken + 1)->type == TokenType::IDENTIFIER;
		 if (isDoubleDot)
		 {

		 }
		 else if (isIdentifier)
		 {
			 currentToken += 1;
			 currentScope = &forNode->initScope;
			 BeginBlock();
			 // init node
			 forNode->init = Declaration(currentToken);
			 
			 // check and action node
			 forNode->condition = LogicalOr(currentToken);
			 //currentToken += 1;
			 Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");

			 forNode->action= Statement(currentToken);
			 forNode->body = EatBlock(currentToken);
			 EndBlock();
		 }
		 return forNode;
	 }
	 else if (currentToken->type == TokenType::LEFT_BRACE)
	 {
		
		 auto scope = EatBlock(currentToken);
		 return scope;
	 }
	 auto expr = EqualOp(currentToken);
	 //Error(TokenType::SEMICOLON, currentToken, "Expected ; at the end of expression");
	 //currentToken++;
	 return expr;
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
std::unique_ptr<Node> AST::ParseExpression( Token*& currentToken)
{

	auto tree = Statement(currentToken);
	return tree;
}
bool AST::Build(Token*& firstToken)
{
	tree = ParseExpression(firstToken);
	return true;
}

void AST::TypeCheck(VirtualMachine& vm)
{
	TypeCheck(tree.get(), vm);
}
bool IsCastable(ValueType to, ValueType from)
{
	if (to == from) return true;
	if (to == ValueType::INT && from == ValueType::FLOAT)
	{
		return true;
	}
	else if (to == ValueType::FLOAT && from == ValueType::INT)
	{
		return true;
	}
	return false;
}

TokenType AST::TypeCheck(Node* node, VirtualMachine& vm)
{
	TokenType childType = TokenType::END;
	TokenType childType1 = TokenType::END;
	if (node->type == TokenType::BLOCK)
	{
		auto block = static_cast<Scope*>(node);
		currentScopes.push_back(block);
		BeginBlock();
		for (auto& e : block->expressions)
		{
			TypeCheck(e.get(), vm);
		}
		EndBlock();
		currentScopes.pop_back();
		return TokenType::BLOCK;
	}
	if (node->type == TokenType::FOR)
	{
		auto forNode = static_cast<For*>(node);
		//currentScope = &forNode->initScope;
		currentScopes.push_back(&forNode->initScope);
		TypeCheck(forNode->init.get(), vm);
		TypeCheck(forNode->condition.get(), vm);
		TypeCheck(forNode->action.get(), vm);
		TypeCheck(forNode->body.get(), vm);
		currentScopes.pop_back();

		return TokenType::FOR;
	}
	auto expr = static_cast<Expression*>(node);
	if (expr->left != nullptr)
	{
		childType = TypeCheck(expr->left.get(), vm);
	}
	if (expr->right != nullptr)
	{
		childType1 = TypeCheck(expr->right.get(),vm);
	}
	auto& globalsType = vm.GetGlobalsType();
	auto& globals = vm.GetGlobals();


	// determine what kind of type operations returns 
	bool areChildren = childType != TokenType::END && childType1 != TokenType::END;
	bool isTermOp = expr->type==TokenType::PLUS || expr->type == TokenType::STAR ||
		expr->type == TokenType::SLASH || expr->type == TokenType::MINUS;
	bool isTermOpEqual = expr->type == TokenType::PLUS_EQUAL ||
		expr->type == TokenType::SLASH_EQUAL ||
		expr->type == TokenType::MINUS_EQUAL ||
		expr->type == TokenType::STAR_EQUAL ;
	if(areChildren&& isTermOp)
	{
		if (childType1 == TokenType::INT_LITERAL && childType == TokenType::INT_LITERAL)
		{
			expr->value = ValueContainer{ ValueType::INT };
			return TokenType::INT_LITERAL;
		}
		expr->value = ValueContainer{ ValueType::FLOAT };
		return TokenType::FLOAT_LITERAL;
	}
	else if (childType != TokenType::END && isTermOpEqual)
	{
		if (childType == TokenType::INT_LITERAL)
		{
			expr->value = ValueContainer{ ValueType::INT };
			return TokenType::INT_LITERAL;
		}
		expr->value = ValueContainer{ ValueType::FLOAT };
		return TokenType::FLOAT_LITERAL;
	}
	
	
	
	
	if (expr->type == TokenType::EQUAL)
	{
		auto leftChild = expr->left->AsMut<Expression>();
		if (childType == TokenType::DEDUCE)
		{
			if (expr->depth > 0)
			{
				leftChild->value.type = LiteralToType(childType1);
				auto str = &leftChild->value.As<String&>();
				Entry* entry;
				for (auto scope : currentScopes)
				{
					if (!scope->types.IsExist(str->GetStringView())) continue;
					entry = scope->types.Get(str->GetStringView());
				}
				entry->value.type = leftChild->value.type;
			}
			else
			{
				leftChild->value.type = LiteralToType(childType1);
				auto str = &leftChild->value.As<String&>();
				auto entry = globalsType.Get(str->GetStringView());
				entry->value.type = leftChild->value.type;
				entry = globals.Get(str->GetStringView());
				entry->value.type = leftChild->value.type;
			}

		}
		return childType1;
	}
	if (expr->type == TokenType::IDENTIFIER)
	{
		if (expr->depth > 0)
		{
			
			auto str = &expr->value.As<String&>();;
			Entry* entry;
			for (auto scope : currentScopes)
			{
				if (!scope->types.IsExist(str->GetStringView())) continue;
				entry = scope->types.Get(str->GetStringView());
			}
			return TypeToLiteral(entry->value.type);
		}
		auto str = &expr->value.As<String&>();
		auto entry = globalsType.Get(str->GetStringView());
		assert(entry->key != nullptr);
		return TypeToLiteral(entry->value.type);
	}
	bool isUnary = expr->type == TokenType::MINUS || expr->type == TokenType::MINUS_MINUS
		|| expr->type == TokenType::PLUS_PLUS;
	if (isUnary && childType != TokenType::END)
	{
		return childType;
	}
	return expr->type;

}


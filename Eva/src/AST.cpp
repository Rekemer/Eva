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
	auto nextToken = (currentToken + 1);
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH;
	if (isMultiplication)
	{
		currentToken++;
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
	 default:
		 assert(false);
		 break;
	 }
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
	auto nextToken = currentToken + 1;
	 bool isDouble = nextToken->type == TokenType::MINUS_MINUS || nextToken->type == TokenType::PLUS_PLUS;
	 if (isDouble)
	 {
		 currentToken++;
		 auto prevOp = currentToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 parent->type = prevOp;
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
	else if (currentToken->type == TokenType::IDENTIFIER)
	{
		if (scopeDepth > 0)
		{
			auto& variableName = currentToken->value.As<String&>();
			node->value = ValueContainer((Object*)&variableName);
			
		}
		else
		{
			auto str = currentToken->value.As<String&>();
			auto& table = vm->GetGlobals();
			auto entry = table.Get(str.GetStringView());
			if (entry->key == nullptr)
			{
				m_Panic = true;
				std::cout << "ERROR[" << (currentToken)->line << "]: " <<
					"The name " << str << " is used but not declared " << std::endl;
			}
			auto variableName = entry->key;
			node->value = ValueContainer((Object*)variableName);
		}
		
	}
	else if (currentToken->type == TokenType::LEFT_PAREN)
	{
		currentToken += 1;
		auto node = ParseExpression(currentToken);
		if (currentToken->type != TokenType::RIGHT_PAREN)
		{
			std::cout << "error: expected  )\n";
		}
		return node;
	}
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
	auto nextToken = (currentToken + 1);
	bool isSum = nextToken->type == TokenType::PLUS ||
		nextToken->type == TokenType::MINUS;
	if (isSum)
	{

		auto operation = nextToken->type;
		auto parent = std::make_unique<Expression>();
		parent->line = currentToken->line;

		currentToken += 2;
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
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::LESS ||
		 nextToken->type == TokenType::LESS_EQUAL || nextToken->type == TokenType::GREATER ||
		 nextToken->type == TokenType::GREATER_EQUAL || nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 2;
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
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::EQUAL_EQUAL ||
		 nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 2;
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
	 auto nextToken = (currentToken + 1);
	 bool isAnd = nextToken->type == TokenType::AND;
	 if (isAnd)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 2;
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
	 bool isVariable = currentToken->type == TokenType::IDENTIFIER;
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
		 currentToken += 2;
		 parent->right = LogicalOr(currentToken);
		 currentToken++;
		 return parent;
	 }
	 return Equal(currentToken);

 }
 std::unique_ptr<Node> AST::Equal(Token*& currentToken)
 {
	 bool isVariable = currentToken->type == TokenType::IDENTIFIER;
	 bool isDeclaration = (currentToken+1)->type == TokenType::COLON;
	 auto varToken = currentToken;
	 auto isAssignment = (currentToken + 1)->type == TokenType::EQUAL;
	 Token* eqToken;

	 

	 if (isDeclaration)
	 {
		auto& table = vm->GetGlobals();
		auto& globalsType = vm->GetGlobalsType();
		auto& str= currentToken->value.As<String&>();

		auto entry = table.Get(str.GetStringView());
		if (entry->key == nullptr)
		{
			auto node = std::make_unique<Expression>();
			node->line = currentToken->line;
			node->type = currentToken->type;

			// if there are tokens that correspond to declaration
			// number :int = 2; number :=2;
			auto declaredType = (currentToken + 2)->type;
			auto [isType,type]= IsVariableType(declaredType);
			if (isType)
			{
				auto isEqualSign = (currentToken + 3)->type == TokenType::EQUAL;
				if (isEqualSign)
				{
					node->type = TokenType::EQUAL;
					// define a local variable
					if (scopeDepth > 0)
					{
						scopeDeclarations++;
						node->left = LogicalOr(currentToken);
						auto leftExpression = static_cast<Expression*>(node->left.get());
						leftExpression->value.type = LiteralToType(type);
						currentToken += 4;
						node->right = LogicalOr(currentToken);
						currentToken ++;

					}
					// define global variable
					else
					{
						globalsType.Add(str.GetStringView(), LiteralToType(declaredType));
						// to note global that variable is declared, so that in value it can be used
						auto variableName = table.Add(str.GetStringView(), ValueContainer{})->key;
						// it will initialize node with the name of a variable
						node->left = LogicalOr(currentToken);
						auto leftExpression = static_cast<Expression*>(node->left.get());
						leftExpression->value.type = LiteralToType(type);
						//node->left->value = ValueContainer((Object*)variableName);
						currentToken += 4;
						node->depth = 0;
						node->right = LogicalOr(currentToken);
						currentToken ++;
					}
				}

			}
			// deduce type
			else
			{
				auto isEqualSign = (currentToken + 2)->type == TokenType::EQUAL;
				if (isEqualSign)
				{

					assert(false && "deduction of types is not supported yet");
					auto variableName = table.Add(str.GetStringView(), ValueContainer{})->key;
					node->left = LogicalOr(currentToken);
					//node->left->value = ValueContainer((Object*)variableName);

					currentToken += 3;
					node->right = LogicalOr(currentToken);
					currentToken ++;

				}
				else
				{
					assert("No equal sign " && false);
				}

			}

			if ((currentToken)->type == TokenType::SEMICOLON)
			{
				// commented so we can run expression tests
				//currentToken += 1;
			}
			else
			{
				m_Panic = true;
				std::cout << "ERROR[" << (currentToken )->line << "]: Expected ; at the end of expression\n";
			}

			return node;


		}
		 
	 }
	 else if (isAssignment)
	 {
		 auto node = std::make_unique<Expression>();
		 auto identiferToken = currentToken;
		 node->line = currentToken->line;
		 node->type = currentToken->type;
		 node->type = TokenType::EQUAL;
		 node->left = LogicalOr(currentToken);
		 currentToken += 2;
		 node->right = LogicalOr(currentToken);
		 return node;
	 }
	 return LogicalOr(currentToken);
 }

 std::unique_ptr<Node> AST::LogicalOr(Token*& currentToken)
 {
	 auto left = LogicalAnd(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isOr = nextToken->type == TokenType::OR;
	 if (isOr)
	 {

		 auto operation = nextToken->type;
		 auto parent = std::make_unique<Expression>();
		 parent->line = currentToken->line;
		 currentToken += 2;
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
	 assert(currentToken->type == TokenType::LEFT_BRACE);
	 auto block = std::make_unique<Scope>();
	 block->type = TokenType::BLOCK;
	 currentToken++;
	 while (currentToken->type != TokenType::RIGHT_BRACE &&
		 currentToken->type != TokenType::END)
	 {
		 auto expression = Statement(currentToken);
		 block->expressions.push_back(std::move(expression));
	 }
	 if (currentToken->type == TokenType::RIGHT_BRACE)
	 {
		 currentToken++;
	 }
	 block->popAmount = scopeDeclarations;
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
	 currentToken += 1;
	 ifnode->right = std::make_unique<Expression>();
	 auto then = EatBlock(currentToken);
	 //ifnode->type= solutionType 
	 // get all then statements in range { } 

	 auto expr = static_cast<Expression*>(ifnode->right.get());
	 expr->right = std::move(then);
	 return ifnode;
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
		 currentToken++;
		 if (currentToken->type != TokenType::SEMICOLON)
		 {
			 std::cout << "ERROR[" << (currentToken)->line << "]: Expected ; at the end of expression\n";
		 }
		 else
		 {
			 currentToken++;
		 }
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
		 currentToken++;
		 whileNode->right = EatBlock(currentToken);
		 return whileNode;
	 }
	 else if (currentToken->type == TokenType::LEFT_BRACE)
	 {
		 BeginBlock(currentToken);
		 auto scope = EatBlock(currentToken);
		 EndBlock(currentToken);
		 return scope;
	 }
	 auto expr = EqualOp(currentToken);
	 currentToken++;
	 return expr;
 }
 
 void AST::BeginBlock(Token*& currentToken)
 {
	 scopeDepth++;
 }
 void AST::EndBlock(Token*& currentToken)
 {
	 scopeDepth--;
	 scopeDeclarations = 0;
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

		for (auto& e : block->expressions)
		{
			TypeCheck(e.get(), vm);
		}
		return TokenType::BLOCK;
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
if (expr->type == TokenType::IDENTIFIER)
{
	//declare a variable
	if (childType != TokenType::END)
	{
		auto str = (String*)expr->value.As<Object*>();
		auto entry = globalsType.Get(str->GetStringView());
		if (entry->key!= nullptr)
		{	// already know type
			auto childValueType = LiteralToType(childType);
			if (!IsCastable(entry->value.type,childValueType) )
			{
				m_Panic = true;
				std::cout << "line [ " << expr->line <<" ]: Cannot cast " << ValueToStr(childValueType)<< 
					" to " << ValueToStr(entry->value.type) << std::endl;
			}
		}
		else
		{
			globalsType.Add(str->GetStringView(), LiteralToType (childType));
		}
	}
	
}
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
	return childType1;
}
if (expr->type == TokenType::IDENTIFIER)
{
	auto str = (String*)expr->value.As<Object*>();
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


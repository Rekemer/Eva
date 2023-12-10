#include"AST.h"
#include<iostream>
#include<cassert>
#include "VirtualMachine.h"
Expression::Expression(Expression&& e)
{
	childrenCount = e.childrenCount;
	left = e.left;
	right= e.right;
	type =  e.type;
	value = std::move(e.value);
	e.left = e.right = nullptr;
	e.childrenCount = 0;
}
 Expression* AST::Factor( Token*& currentToken)
{
	auto left = UnaryOp(currentToken);
	auto nextToken = (currentToken + 1);
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH;
	if (isMultiplication)
	{

		auto operation = nextToken->type;
		currentToken += 2;
		auto right = Factor(currentToken);
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
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
		 return ValueType::INT;
		 break;
	 case TokenType::FLOAT_LITERAL:
		 return ValueType::FLOAT;
		 break;
	 case TokenType::STRING_LITERAL:
		 return ValueType::OBJ;
		 break;
	 case TokenType::FALSE:
	 case TokenType::TRUE:
		 return ValueType::BOOL;
		 break;
	 default:
		 assert(false);
		 break;
	 }
 }
 
 Expression* AST::UnaryOp( Token*&  currentToken)
{
	bool isUnary = currentToken->type == TokenType::MINUS  || currentToken->type == TokenType::BANG? true : false;
	if (isUnary)
	{
		auto prevOp = currentToken->type;
		currentToken++;
		Expression* parent = new Expression();
		parent->type = prevOp;
		parent->left = UnaryOp(currentToken);
		parent->value.type = LiteralToType(parent->left->type);
		return parent;
	}

	return Value(currentToken);
}

 Expression* AST::Value( Token*& currentToken)
{
	Expression* node = new Expression();
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

		// check if variable is declared  or not
		auto& table = vm->GetGlobals();
		// why it doesn work?
		//auto obj = node->value.As<Object>();
		auto obj = currentToken->value.As<Object*>();
		auto str = static_cast<String*>(obj);
		
		// declaration of varaible
		// usage of declared variable
		// usage of unknown identifier	
		if (!table.IsExist(str->GetStringView()))
		{
			// if there are tokens that correspond to declaration
			// number :int = 2; number :=2;
			auto isDeclaration = (currentToken + 1)->type == TokenType::COLON;
			if (isDeclaration)
			{
				auto isType = IsVariableType((currentToken + 2)->type);
				if (isType)
				{
					auto isEqualSign = (currentToken + 3)->type == TokenType::EQUAL;
					if (isEqualSign)
					{
						currentToken += 3;
						node->left = ParseExpression(currentToken);

						auto variableName = table.Add(str->GetStringView(), ValueContainer{})->key;
						
						node->value = ValueContainer((Object*)variableName);
					}

				}
				// deduce type
				else
				{
					auto isEqualSign = (currentToken + 2)->type == TokenType::EQUAL;
					if (isEqualSign)
					{
						currentToken += 2;
						node->left = ParseExpression(currentToken);

						auto variableName = table.Add(str->GetStringView(), ValueContainer{})->key;
						node->value = ValueContainer((Object*)variableName);
					}
					else
					{
						assert("No equal sign " && false);
					}
					
				}
				
				if ((currentToken )->type == TokenType::SEMICOLON)
				{

					//currentToken += 1;
				}
				else
				{
					assert("Handle semicolon" && false);
				}


			}
			// we reading a variable
			//node->value = ValueContainer((Object*)str);
		}
		else
		{

			/*TokenType tokenType;
			if (value.type == ValueType::BOOL)
			{
				tokenType = TokenTy
			}
			else if (value.type == ValueType::INT)
			{

			}
			else if (value.type == ValueType::FLOAT)
			{

			}
			else if (value.type == ValueType::OBJ)
			{

			}*/
			node->value = ValueContainer((Object*)str );

		}
	}
	else if (currentToken->type == TokenType::LEFT_PAREN)
	{
		currentToken += 1;
		node = ParseExpression(currentToken);
		if (currentToken->type != TokenType::RIGHT_PAREN)
		{
			std::cout << "error: expected  )\n";
		}
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
		Print(tree->left, level + 1);

		// Recursively print the right subtree with increased indentation
		Print(tree->right, level + 1);
	}
}
 Expression* AST::Term( Token*& currentToken)
{
	auto left = Factor(currentToken);
	auto nextToken = (currentToken + 1);
	bool isSum = nextToken->type == TokenType::PLUS ||
		nextToken->type == TokenType::MINUS;
	if (isSum)
	{

		auto operation = nextToken->type;
		currentToken += 2;
		auto right = Term(currentToken);
		auto parent = new Expression();
		parent->left = left;
		parent->right = right;
		parent->type = operation;
		return parent;

	}
	return left;
}

 Expression* AST::Comparison( Token*& currentToken)
 {
	 auto left = Term(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::LESS ||
		 nextToken->type == TokenType::LESS_EQUAL || nextToken->type == TokenType::GREATER ||
		 nextToken->type == TokenType::GREATER_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = Comparison(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 Expression* AST::Equality(Token*& currentToken)
 {
	 auto left = Comparison(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isCompare = nextToken->type == TokenType::EQUAL_EQUAL ||
		 nextToken->type == TokenType::BANG_EQUAL;
	 if (isCompare)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = Equality(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 Expression* AST::LogicalAnd(Token*& currentToken)
 {
	 auto left = Equality(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isAnd = nextToken->type == TokenType::AND;
	 if (isAnd)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = LogicalAnd(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 
 Expression* AST::Equal(Token*& currentToken)
 {
	 bool isEqual = currentToken->type == TokenType::EQUAL;
	 if (isEqual)
	 {

		 auto operation = currentToken->type;
		 currentToken += 1;
		 auto left = LogicalOr(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->type = operation;
		 return parent;

	 }
	 return LogicalOr(currentToken);
 }

 Expression* AST::LogicalOr(Token*& currentToken)
 {
	 auto left = LogicalAnd(currentToken);
	 auto nextToken = (currentToken + 1);
	 bool isOr = nextToken->type == TokenType::OR;
	 if (isOr)
	 {

		 auto operation = nextToken->type;
		 currentToken += 2;
		 auto right = LogicalOr(currentToken);
		 auto parent = new Expression();
		 parent->left = left;
		 parent->right = right;
		 parent->type = operation;
		 return parent;

	 }
	 return left;
 }

 Expression* AST::Statement(Token*& currentToken)
 {


	 if (currentToken->type == TokenType::PRINT)
	 {
		 currentToken += 1;
		 auto* printNode = new Expression();
		 printNode->type = TokenType::PRINT;
		 printNode->left = LogicalOr(currentToken);
		 currentToken++;
		 if (currentToken->type != TokenType::SEMICOLON)
		 {
			 std::cout << "ERROR[ " << (currentToken-1)->line << " ]: Expected ; at the end of expression\n";
		 }
		 else
		 {
			 currentToken++;
		 }
		 return printNode;
	 }
	 
	 auto* expr = Equal(currentToken);
	 currentToken++;
	 return expr;
 }
 

Expression* AST::ParseExpression( Token*& currentToken)
{

	Expression* tree = Statement(currentToken);
	return tree;
}
bool AST::Build(Token*& firstToken)
{
	tree = std::make_unique<Expression>(std::move(*ParseExpression(firstToken)) );
	return true;
}

void AST::TypeCheck(VirtualMachine& vm)
{
	TypeCheck(tree.get(),vm);
}
#define DETERMINE_TYPE(child1,child2)\

TokenType AST::TypeCheck(Expression* expr, VirtualMachine& vm)
{
	TokenType childType = TokenType::END;
	TokenType childType1 = TokenType::END;

	if (expr->left != nullptr)
	{
		childType = TypeCheck(expr->left,vm);
	}
	if (expr->right != nullptr)
	{
		childType1 = TypeCheck(expr->right,vm);
	}
	if (expr->type == TokenType::IDENTIFIER)
	{
		auto& globalsType = vm.GetGlobalsType();
		//declare a variable
		if (childType != TokenType::END)
		{
			auto str = (String*)expr->value.As<Object*>();
			globalsType.Add(str->GetStringView(), ValueContainer{ LiteralToType (childType)});
		}
		
	}
	bool areChildren = childType != TokenType::END && childType1 != TokenType::END;
	if(areChildren&& (expr->type == TokenType::PLUS || expr->type == TokenType::STAR ||
		expr->type == TokenType::SLASH || expr->type == TokenType::MINUS))
	{
		if (childType1 == TokenType::INT_LITERAL && childType == TokenType::INT_LITERAL)
		{
			expr->value = ValueContainer{ ValueType::INT };
			return TokenType::INT_LITERAL;
		}
		expr->value = ValueContainer{ ValueType::FLOAT };
		return TokenType::FLOAT_LITERAL;
	}
	if (expr->type == TokenType::EQUAL)
	{
		return childType;
	}
	return expr->type;

}


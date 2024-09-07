#include "AST.h"
#include <iostream>
#include <cassert>
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
	auto nextToken = (currentToken );
	bool isMultiplication = nextToken->type == TokenType::STAR ||
		nextToken->type == TokenType::SLASH || 
		nextToken->type == TokenType::PERCENT;
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
	 globalTypes.Add(str->GetStringView(), type);
	 // define global variable
	 table.Add(str->GetStringView(), type);
	 // to note global that variable is declared, so that in value it can be used
	 auto variableName = table.Add(str->GetStringView(), ValueContainer{})->key;
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
	 vm->AddLocal(*str, scopeDepth);
	 node->left = LogicalOr(currentToken);

	 currentScope->types.Add(str->GetStringView(), type);
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
		if (currentToken->type == TokenType::SEMICOLON)
		{
			currentToken++;
		}
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
		auto entry = globalTable.Get(str->GetStringView());
		auto isGlobal = entry->key != nullptr;

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

				auto [isLocalDeclared, index] = vm->IsLocalExist(*str, scopeDepth);
				if (isLocalDeclared)
				{
					// should check whether it is declared variable

					auto variableName = currentToken->value.AsString();
					node->value = ValueContainer(variableName);
					node->depth = scopeDepth;
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

		currentToken++;
		auto right = Term(currentToken);
		parent->left = std::move(left);
		parent->right = std::move(right);
		parent->type = operation;
		return parent;

	}
	return left;
}

 std::unique_ptr<Node> AST::Comparison( Iterator& currentToken)
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

 std::unique_ptr<Node> AST::Equality(Iterator& currentToken)
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

 std::unique_ptr<Node> AST::LogicalAnd(Iterator& currentToken)
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
		 parent->left = LogicalOr((currentToken));
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
	 auto funcValue = globalVariables.Add(name->GetStringView(), LiteralToType(TokenType::FUN))
		 ->value.AsFunc();
	 function->name = name;
	 function->type = TokenType::FUN;
	 currentScope = &function->paramScope;
	 BeginBlock();
	 while (currentToken->type != TokenType::RIGHT_PAREN)
	 {
		 auto arg = Declaration(currentToken);
		 auto name = vm->LastLocal();
		 auto declaredType = currentScope->types.Get(name)->value.type;
		 funcValue->argTypes.push_back(declaredType);
		 //auto type = currentScope->

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
		globalTypes.Add(function->name->GetStringView(), ValueType::NIL);
	 }
	 else
	 {
		Error(TokenType::COLON,currentToken,"Function must declare return type");
		globalTypes.Add(function->name->GetStringView(), LiteralToType(currentToken->type));
		currentToken++;
	 }
	 function->body = EatBlock(currentToken);
	 auto body = function->body->AsMut<Scope>();
	 
	 for (auto& arg : function->arguments)
	 {
		 auto expr = arg->As<Expression>()->left->As<Expression>();
		 auto name = expr->value.AsString()->GetStringView();
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

	 auto str = *currentToken->value.AsString();
	 auto declaredType = (currentToken + 2)->type;
	 auto isEqualSign = (currentToken + 3)->type == TokenType::EQUAL;
	 auto entry = table.Get(str.GetStringView());
	 auto [isType, type] = IsVariableType(declaredType);
	 auto node = std::make_unique<Expression>();
	 node->type = TokenType::DECLARE;
	 node->depth = scopeDepth;
	 if (entry->key == nullptr && scopeDepth == 0)
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
 bool IsBinaryOp(TokenType type)
 {
	 return type == TokenType::PLUS || type == TokenType::STAR ||
		 type == TokenType::SLASH || type == TokenType::MINUS;
 }
 Expression* AST::FoldConstants(Expression* expr)
 {
	 if (!expr)
	 {
		 return {};
	 }
	 if (expr->type == TokenType::INT_LITERAL || expr->type == TokenType::FLOAT_LITERAL)
	 {
		 return expr;
	 }
	 if (expr->type == TokenType::MINUS && !expr->right && isLiteral(expr->left->type))
	 {
		 auto left = expr->left->AsMut<Expression>();

		 // could we perhaps do it without allocation?
		 auto exprInverted = new Expression();
		 exprInverted->type = left->type;
		 exprInverted->value = left->value;
		 exprInverted->value.Negate();
		 return exprInverted;
	 }
	 // check if both constant
		// replace subtree with the new node
	 if (expr->type == TokenType::IDENTIFIER )
	 {
		 return {};
	 }
	 auto isBinaryOp = IsBinaryOp(expr->type);
	 if (isBinaryOp)
	 {
		 // recurse so we can have series of constant + constant + ...
		 auto left = FoldConstants(static_cast<Expression*>(expr->left.get()));
		 if (IsBinaryOp(expr->left->type))
		 {
			 expr->left = std::unique_ptr<Node>(left);
		 }
		 auto right = FoldConstants(static_cast<Expression*>(expr->right.get()));
		 if (IsBinaryOp(expr->right->type))
		 {
			 expr->right = std::unique_ptr<Node>(right);
		 }
		 auto isLitL = isLiteral(left ->type);
		 auto isLitR = isLiteral(right->type);
		 // case constant + constant 
		 if (isLitL && isLitR)
		 {
			 auto node = new Expression();
			 auto newType = (left->type == TokenType::FLOAT_LITERAL || right->type == TokenType::FLOAT_LITERAL)
				 ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;
			 switch (expr->type)
			 {
			 case TokenType::PLUS:
				 node->value = ValueContainer::Add(left->value, right->value, *vm);
				 node->type = newType;
				 break;
			 case TokenType::MINUS:
				 node->value = ValueContainer::Substract(left->value, right->value);
				 node->type = newType;
				 break;
				 //case TokenType::STAR:
				 //	node->value = left->value * right->value;
				 //	break;
				 //case TokenType::SLASH:
				 //	node->value = left->value / right->value;
				 //	break;
			 default:
				 break;
			 }
			 return node;
		 }
		 expr->left = std::unique_ptr<Node>(left);
		 expr->right = std::unique_ptr<Node>(right);
		
	 }
	return {};
 }
 void AST::Fold()
 {
	 auto expr = tree->AsMut<Expression>();
	 auto left = expr->left.get()->AsMut<Expression>();
	 auto right = expr->right.get()->AsMut<Expression>();
	 
	 auto newNodeLeft = FoldConstants(left);
	 if (newNodeLeft)
	 {
		 expr->left = std::unique_ptr<Node>(static_cast<Node*>(newNodeLeft));
	 }
	 auto newNodeRight = FoldConstants(right);
	 if (newNodeRight)
	 {
		 expr->right= std::unique_ptr<Node>(static_cast<Node*>(newNodeRight));
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
		 auto name = currentToken->value.AsString()->GetStringView();
		 auto check = isDeclarared(name);
		 if (check) return {};
		 return DeclareVariable(currentToken);
	 }
	 else if (isFunc)
	 {	
		 auto name = (currentToken+1)->value.AsString()->GetStringView();
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
 std::unique_ptr<Node> AST::LogicalOr(Iterator& currentToken)
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

 std::unique_ptr<Node> AST::EatBlock(Iterator& currentToken)
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
 std::unique_ptr<Node> AST::EatIf(Iterator& currentToken)
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
 void AST::TravelSemicolon(Iterator& currentToken)
 {
	 while (currentToken->type != TokenType::SEMICOLON)
	 {
		 currentToken++;
	 }
	 currentToken++;
 }
 void AST::ErrorTypeCheck(int line, const char* str)
 {
	 m_Panic = true;
	 std::cout << "ERROR[" << line << "] " << str << std::endl;
 }
 void AST::ErrorTypeCheck(int line, std::stringstream& ss)
 {
	 m_Panic = true;
	 std::cout << "ERROR[" << line << "] " << ss.str() << std::endl;
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
		 m_ParseLoops.push(true);
		 auto whileNode = std::make_unique<Expression>();
		 whileNode->type = TokenType::WHILE;
		 whileNode->line = currentToken->line;
		 currentToken++;
		 whileNode->left = LogicalOr(currentToken);
		 whileNode->right = EatBlock(currentToken);
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

			 currentScope = &(forNode->initScope);
			 BeginBlock();

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
				 forNode->body = EatBlock(currentToken);
 				 forNode->initScope.popAmount = scopeDeclarations.size() > 0 ? scopeDeclarations.top() : 0;
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
			 // so we can create i Iterator&
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
			 forNode->initScope.popAmount = scopeDeclarations.size() > 0 ?
				 scopeDeclarations.top() : 0;
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
		
		 auto scope = EatBlock(currentToken);
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
bool AST::Build(Iterator& firstToken)
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
	if (node->type == TokenType::IF) return TokenType::END;
	if (node->type == TokenType::LEFT_PAREN)
	{
		auto call = static_cast<Call*>(node);

		auto& globals = vm.GetGlobals();
		auto funcValue = globals.Get(call->name->GetStringView())
			->value.AsFunc();

		for ( auto i = 0; i < call->args.size(); i++)
		{
			auto& arg = call->args[i];
			auto type = LiteralToType(TypeCheck(arg.get(), vm));
			auto declType = funcValue->argTypes[i];
			// check declared type and real passed type
			auto castable = IsCastable(declType, type);
			if (!castable)
			{
				std::stringstream ss;
				ss << "The " << i << " declared argument's type is " << ValueToStr(declType) <<
					", but the passed type is " << ValueToStr(type);
				ErrorTypeCheck(call->line,ss);
				return TokenType::NIL;
			}

			// we need to compare type of arguments 
			// with the the type of parameters
			
		}
		auto name = call->name->GetStringView();
		auto entry = vm.GetGlobalsType().Get(name);
		std::stringstream ss;
		assert(entry->key != nullptr);
		return TypeToLiteral(entry->value.type);
	}
	if (node->type == TokenType::RETURN)
	{

		auto ret = TypeCheck(static_cast<Expression*>(node)->left.get(), vm);
		if (ret == TokenType::NIL)
		{
			ErrorTypeCheck(node->line, "Cannot return NIL value");
		}
		return ret;
	}
	if (node->type == TokenType::FUN)
	{
		auto fun = static_cast<FunctionNode*>(node);
		auto entry = vm.GetGlobalsType().Get(fun->name->GetStringView());
		auto actualType = TypeCheck(fun->body.get(), vm);
		assert(entry->key != nullptr);
		if (entry->value.type != ValueType::NIL)
		{
			auto declaredType = TypeToLiteral(entry->value.type);
			assert(declaredType == actualType);
			return declaredType;
		}
		return TokenType::NIL;
	}
	if (node->type == TokenType::CONTINUE || node->type == TokenType::BREAK)
	{
		return node->type;
	}
	if (node->type == TokenType::BLOCK)
	{
		auto block = static_cast<Scope*>(node);
		currentScopes.push_back(block);

		BeginBlock();
		auto ret = TokenType::BLOCK;
		for (auto& e : block->expressions)
		{
			auto type = TypeCheck(e.get(), vm);
			if (e->type == TokenType::RETURN)
			{
				ret = type;
			}
		}
		EndBlock();
		currentScopes.pop_back();
		return ret;
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
	bool isTermOp = IsBinaryOp(expr->type);
	bool isTermOpEqual = expr->type == TokenType::PLUS_EQUAL ||
		expr->type == TokenType::SLASH_EQUAL ||
		expr->type == TokenType::MINUS_EQUAL ||
		expr->type == TokenType::STAR_EQUAL ;
	if(areChildren&& isTermOp)
	{

		if (childType1 == TokenType::NIL || childType == TokenType::NIL)
		{
			ErrorTypeCheck(expr->line,"NIL operand in binary operation");
			return TokenType::NIL;
		}
		if (childType1 == TokenType::INT_LITERAL && childType == TokenType::INT_LITERAL)
		{
			expr->value = ValueContainer{ ValueType::INT };
			return TokenType::INT_LITERAL;
		}
		else if (childType1 == TokenType::STRING_LITERAL || childType == TokenType::STRING_LITERAL)
		{
			//ErrorTypeCheck(expr->line,"Strings cannot participate in binary operations yet ");

			return TokenType::STRING_LITERAL;
		}
		expr->value = ValueContainer{ ValueType::FLOAT };
		return TokenType::FLOAT_LITERAL;
	}
	else if (childType1 != TokenType::END && isTermOpEqual)
	{
		auto type = LiteralToType(childType);
		auto type1 = LiteralToType(childType1);
		if (!IsCastable(type, type1))
		{
			std::stringstream ss;
			ss << "cannot cast " << ValueToStr(type1) << " to " << ValueToStr(type);
			ErrorTypeCheck(node->line, ss);
			return TokenType::NIL;
		}
		if (childType1 == TokenType::NIL)
		{
			ErrorTypeCheck(expr->line, "cannot use NIL value");
			return TokenType::NIL;
		}
		if (childType1 == TokenType::INT_LITERAL)
		{
			expr->value = ValueContainer{ ValueType::INT };
			return TokenType::INT_LITERAL;
		}
		expr->value = ValueContainer{ ValueType::FLOAT };
		return TokenType::FLOAT_LITERAL;
	}
	
	
	
	
	if (expr->type == TokenType::DECLARE)
	{
		auto leftChild = expr->left->AsMut<Expression>();
		if (childType1 == TokenType::NIL)
		{
			ErrorTypeCheck(expr->line, "cannot assign NIL value");
			return TokenType::NIL;
		}
		if (childType == TokenType::DEDUCE)
		{
			if (expr->depth > 0)
			{
				auto str = leftChild->value.AsString();
				Entry* entry = nullptr;
				for (auto scope : currentScopes)
				{
					if (!scope->types.IsExist(str->GetStringView())) continue;
					entry = scope->types.Get(str->GetStringView());
				}
				assert(entry != nullptr);
				entry->value.UpdateType(LiteralToType(childType1));
			}
			else
			{
				auto str = leftChild->value.AsString();
				auto entry = globalsType.Get(str->GetStringView());
				entry->value.UpdateType(LiteralToType(childType1));
			}

		}
		return childType1;
	}
	if (expr->type == TokenType::IDENTIFIER)
	{
		if (expr->depth > 0)
		{
			
			auto str = expr->value.AsString();
			Entry* entry;
			for (auto scope : currentScopes)
			{
				if (!scope->types.IsExist(str->GetStringView())) continue;
				entry = scope->types.Get(str->GetStringView());
			}
			return TypeToLiteral(entry->value.type);
		}
		auto str = expr->value.AsString();
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


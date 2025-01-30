#include"Tokens.h"
namespace Eva
{
	std::map<TokenType, std::string> tokenStrings = {
			{TokenType::LEFT_PAREN, "LEFT_PAREN"},
			{TokenType::RIGHT_PAREN, "RIGHT_PAREN"},
			{TokenType::LEFT_BRACE, "LEFT_BRACE"},
			{TokenType::RIGHT_BRACE, "RIGHT_BRACE"},
			{TokenType::COMMA, "COMMA"},
			{TokenType::DOT, "DOT"},
			{TokenType::MINUS, "MINUS"},
			{TokenType::PERCENT, "PERCENT"},
			{TokenType::PLUS, "PLUS"},
			{TokenType::SEMICOLON, "SEMICOLON"},
			{TokenType::SLASH, "SLASH"},
			{TokenType::STAR, "STAR"},
			{TokenType::BANG, "BANG"},
			{TokenType::BANG_EQUAL, "BANG_EQUAL"},
			{TokenType::EQUAL, "EQUAL"},
			{TokenType::EQUAL_EQUAL, "EQUAL_EQUAL"},
			{TokenType::GREATER, "GREATER"},
			{TokenType::GREATER_EQUAL, "GREATER_EQUAL"},
			{TokenType::LESS, "LESS"},
			{TokenType::LESS_EQUAL, "LESS_EQUAL"},
			{TokenType::IDENTIFIER, "IDENTIFIER"},
			{TokenType::STRING_LITERAL, "STRING"},
			{TokenType::INT_LITERAL, "INT"},
			{TokenType::FLOAT_LITERAL, "FLOAT"},
			{TokenType::AND, "AND"},
			{TokenType::CLASS, "CLASS"},
			{TokenType::ELSE, "ELSE"},
			{TokenType::FALSE, "FALSE"},
			{TokenType::FOR, "FOR"},
			{TokenType::FUN, "FUN"},
			{TokenType::IF, "IF"},
			{TokenType::NIL, "NIL"},
			{TokenType::OR, "OR"},
			{TokenType::RETURN, "RETURN"},
			{TokenType::SUPER, "SUPER"},
			{TokenType::THIS, "THIS"},
			{TokenType::TRUE, "TRUE"},
			{TokenType::VAR, "VAR"},
			{TokenType::WHILE, "WHILE"},
			{TokenType::ERROR, "ERROR"},
			{TokenType::END, "END"},
			{TokenType::DECLARE, "DECLARE"},
			{TokenType::JUMP, "JUMP"},
			{TokenType::BRANCH, "BRANCH"},
			{TokenType::PHI, "PHI"},
			{TokenType::BLOCK, "BLOCK"},
			{TokenType::PLUS_PLUS, "PLUS_PLUS"},
			{TokenType::MINUS_MINUS, "MINUS_MINUS"},
			{TokenType::BRANCH_ELIF, "BRANCH_ELIF"},
			{TokenType::BRANCH_WHILE, "BRANCH_WHILE"},
			{TokenType::JUMP_BRANCH, "JUMP_BRANCH"},
			{TokenType::JUMP_WHILE, "JUMP_WHILE"},
			{TokenType::CONTINUE, "CONTINUE"},
			{TokenType::BREAK, "BREAK"},
			{TokenType::JUMP_FOR, "JUMP_FOR"},
			{TokenType::BRANCH_FOR, "BRANCH_FOR"},
	};
	std::string tokenToString(TokenType token) {


		auto it = tokenStrings.find(token);
		if (it != tokenStrings.end()) {
			return it->second;
		}
		else {
			return "UNKNOWN_TOKEN";
		}
	}
}
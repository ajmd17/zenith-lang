#ifndef __PARSER_H__
#define __PARSER_H__

#include <vector>
#include <string>
#include <memory>

#include "ast.h"
#include "tokens.h"
#include "state.h"
#include "src_location.h"

namespace zenith
{
	class Parser
	{
	private:

		SourceLocation getCurrentLocation();

		Token *peek(int n = 0);
		Token *read();

		bool match(TokenType type, int n = 0);
		bool match(TokenType type1, TokenType type2);
		bool match(TokenType type, const std::string &str);
		bool match_read(TokenType type);
		bool match_read(TokenType type, Token *&token);
		bool match_read(TokenType type, const std::string &str);
		Token *expect_read(TokenType type);
		Token *expect_read(TokenType type, const std::string &str);

		int getOpPrecedence();

		std::unique_ptr<AstNode> parseImport();
		std::unique_ptr<AstNode> parseStatement();
		std::unique_ptr<AstNode> parseVarDeclaration();
		std::unique_ptr<AstNode> parseTerm();
		std::unique_ptr<AstNode> parseExpression(bool shouldClearStack = false);
		std::unique_ptr<AstNode> parseParenthesis();
		std::unique_ptr<AstNode> parseInteger();
		std::unique_ptr<AstNode> parseFloat();
		std::unique_ptr<AstNode> parseIdentifier();
		std::unique_ptr<AstNode> parseString();
		std::unique_ptr<AstNode> parseTrue();
		std::unique_ptr<AstNode> parseFalse();
		std::unique_ptr<AstNode> parseNull();
		std::unique_ptr<AstNode> parseBinaryOperation(int exprPrec, std::unique_ptr<AstNode> left);
		std::unique_ptr<AstNode> parseUnaryOperation();
		std::unique_ptr<AstNode> parseBlock();
		std::unique_ptr<AstNode> parseFunctionDefinition();
		std::unique_ptr<AstNode> parseIfStatement();
		std::unique_ptr<AstNode> parseReturnStatement();
		std::unique_ptr<AstNode> parseForLoop();

	public:
		ParserState state;

		Parser(std::vector<Token> tokens, LexerState lexerState);
		std::unique_ptr<ModuleAst> parse();
	};
}

#endif
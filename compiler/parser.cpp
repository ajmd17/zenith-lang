#include "parser.h"

#include <iostream>
#include <stack>
#include <algorithm>
#include <functional>

#include "keywords.h"
#include "operators.h"

namespace zenith
{
	Parser::Parser(std::vector<Token> tokens, LexerState lexerState)
	{
		state.tokens = tokens;
		state.errors = lexerState.errors;
		state.filepath = lexerState.filepath;
	}

	std::unique_ptr<ModuleAst> Parser::parse()
	{
		std::unique_ptr<ModuleAst> unit = nullptr;

		if (!match_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_MODULE)))
			state.errors.push_back({ ErrorType::EXPECTED_MODULE_DECLARATION, getCurrentLocation(), state.filepath });
		else
		{
			Token *ident = expect_read(TokenType::TK_IDENTIFIER);
			if (ident)
			{
				std::string moduleName = ident->value;

				unit = std::make_unique<ModuleAst>(getCurrentLocation(), moduleName);
				while (state.position < state.tokens.size())
				{
					unit->addChild(parseStatement());
				}

				return unit;
			}
		}

		unit = std::make_unique<ModuleAst>(getCurrentLocation(), "__anon__");
		return unit;
	}

	SourceLocation Parser::getCurrentLocation()
	{
		if (peek() != nullptr)
			return peek()->location;
		else if (state.tokens.empty())
			return SourceLocation(0, 0, "");
		else
			return peek(-1)->location;
	}

	Token *Parser::peek(int n)
	{
		if (state.position + n < state.tokens.size())
			return &state.tokens[state.position + n];

		return nullptr;
	}

	Token *Parser::read()
	{
		if (state.position >= state.tokens.size())
			return nullptr;
		else
			return &state.tokens[state.position++];
	}

	bool Parser::match(TokenType type, int n)
	{
		if (peek(n) && peek(n)->type == type)
			return true;
		return false;
	}

	bool Parser::match(TokenType type1, TokenType type2)
	{
		if (match(type1) && match(type2, 1))
			return true;
		return false;
	}

	bool Parser::match(TokenType type, const std::string &str)
	{
		if (match(type) && peek()->value == str)
			return true;
		return false;
	}

	bool Parser::match_read(TokenType type)
	{
		if (match(type))
		{
			read();
			return true;
		}

		return false;
	}

	bool Parser::match_read(TokenType type, Token *&token)
	{
		if (match(type))
		{
			token = read();
			return true;
		}
		return false;
	}

	bool Parser::match_read(TokenType type, const std::string &str)
	{
		if (match(type, str))
		{
			read();
			return true;
		}
		return false;
	}

	Token *Parser::expect_read(TokenType type)
	{
		Token *token = nullptr;
		if (!match_read(type, token))
		{
			Token *badToken = this->read();
			if (badToken != nullptr)
			{
				switch (type)
				{
				case TokenType::TK_IDENTIFIER:
					state.errors.push_back(Error(ErrorType::EXPECTED_IDENTIFIER, badToken->location, state.filepath));
					break;
				case TokenType::TK_OPEN_PARENTHESIS:
				case TokenType::TK_CLOSE_PARENTHESIS:
				case TokenType::TK_OPEN_BRACE:
				case TokenType::TK_CLOSE_BRACE:
				case TokenType::TK_OPEN_BRACKET:
				case TokenType::TK_CLOSE_BRACKET:
				case TokenType::TK_SEMICOLON:
				case TokenType::TK_COLON:
				case TokenType::TK_COMMA:
					state.errors.push_back(Error(ErrorType::EXPECTED_TOKEN, badToken->location, state.filepath, Token::asString(type)));
					break;
				default:
					state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, badToken->location, state.filepath, badToken->value));
					break;
				}
			}
			else
				state.errors.push_back(Error(ErrorType::UNEXPECTED_END_OF_FILE, getCurrentLocation(), state.filepath));
		}

		return token;
	}

	Token *Parser::expect_read(TokenType type, const std::string &str)
	{
		Token *token = peek();
		if (!match_read(type, str))
		{
			Token *badToken = this->read();
			if (badToken != nullptr)
				state.errors.push_back(Error(ErrorType::EXPECTED_TOKEN, badToken->location, state.filepath, str));
			else
				state.errors.push_back(Error(ErrorType::UNEXPECTED_END_OF_FILE, getCurrentLocation(), state.filepath));
		}
		return token;
	}

	int Parser::getOpPrecedence()
	{
		auto *current = peek();
		if (!(current && current->type == TokenType::TK_OPERATOR))
			return -1;

		Operator op = getOperator(current->value);
		return getPrecedence(op);
	}

	std::unique_ptr<AstNode> Parser::parseImport()
	{
		auto token = expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_IMPORT));
		std::string localPath = state.filepath.substr(0, state.filepath.find_last_of("/\\")) + "/";

		bool continued = false;

		Token *tk = nullptr;
		bool isModuleImport = true;
		std::string value;
		if (match_read(TokenType::TK_STRING, tk))
		{
			isModuleImport = false;
			value = tk->value;
		}
		else if (match_read(TokenType::TK_IDENTIFIER, tk))
		{
			isModuleImport = true;
			value = tk->value;
		}
		else
			state.errors.push_back({ ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, read()->value });

		auto result = std::make_unique<ImportAst>(getCurrentLocation(), value, localPath, isModuleImport);
		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseStatement()
	{
		if (match(TokenType::TK_KEYWORD))
		{
			const std::string &val = peek()->value;
			if (val == getKeywordStr(Keyword::KW_VAR))
				return parseVarDeclaration();
			/*else if (val == getKeyword(Keyword::KW_CLASS))
			return parseClass();
			else if (val == getKeyword(Keyword::KW_ENUM))
			return parseEnum();*/
			else if (val == getKeywordStr(Keyword::KW_IMPORT))
				return parseImport();
			else if (val == getKeywordStr(Keyword::KW_FUNCTION))
				return parseFunctionDefinition();
			else if (val == getKeywordStr(Keyword::KW_IF))
				return parseIfStatement();
			else if (val == getKeywordStr(Keyword::KW_RETURN))
				return parseReturnStatement();
			else if (val == getKeywordStr(Keyword::KW_FOR))
				return parseForLoop();
			/*else if (val == getKeyword(Keyword::KW_FOREACH))
			return parseForEach();
			else if (val == getKeyword(Keyword::KW_WHILE))
			return parseWhile();
			else if (val == getKeyword(Keyword::KW_BREAK))
			return parseBreak();
			else if (val == getKeyword(Keyword::KW_CONTINUE))
			return parseContinue();
			else if (val == getKeyword(Keyword::KW_DO))
			return parseDoWhile();
			else if (val == getKeyword(Keyword::KW_IMPORT))
			return parseImport();
			else if (val == getKeyword(Keyword::KW_THROW))
			return parseThrow();
			else if (val == getKeyword(Keyword::KW_TRY))
			return parseTry();
			else if (val == getKeyword(Keyword::KW_SUPER))
			return parseSuper();*/
			else
			{
				read();
				state.errors.push_back({ ErrorType::INTERNAL_ERROR, getCurrentLocation(), state.filepath });
			}
		}
		else if (match(TokenType::TK_OPEN_BRACE))
			return parseBlock();
		else if (match_read(TokenType::TK_SEMICOLON))
			return std::move(std::make_unique<StatementAst>(getCurrentLocation()));
		else
		{
			auto node = parseExpression(true);
			if (node == nullptr)
				state.errors.push_back({ ErrorType::ILLEGAL_EXPRESSION, getCurrentLocation(), state.filepath });

			return std::move(node);
		}
	}

	std::unique_ptr<AstNode> Parser::parseVarDeclaration()
	{
		auto token = expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_VAR));
		auto identifier = peek()->value;

		if (peek(1)->type == TokenType::TK_OPERATOR)
		{
			if (peek(1)->value != getOperatorStr(Operator::OP_ASSIGN))
			{
				state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, peek(1)->value));
				return nullptr;
			}

			// parser will continue from here and use parseExpression()
		}
		else
		{
			// no expression following, so eat the identifier
			read();
		}

		auto varDeclAst = std::make_unique<VariableDeclarationAst>(getCurrentLocation(), identifier);
		return std::move(varDeclAst);
	}

	std::unique_ptr<AstNode> Parser::parseBinaryOperation(int exprPrec, std::unique_ptr<AstNode> left)
	{
		while (true)
		{
			int tokenPrec = getOpPrecedence();

			if (tokenPrec < exprPrec)
				return left;

			auto *opToken = expect_read(TokenType::TK_OPERATOR);

			Operator op = getOperator(opToken->value);

			auto right = parseTerm();
			if (!right)
				return nullptr;

			int nextPrec = getOpPrecedence();
			if (tokenPrec < nextPrec)
			{
				right = parseBinaryOperation(tokenPrec + 1, std::move(right));
				if (!right)
					return nullptr;
			}

			left = std::make_unique<BinaryOperationAst>(getCurrentLocation(), std::move(left), std::move(right), op);
		}

		return nullptr;
	}

	std::unique_ptr<AstNode> Parser::parseUnaryOperation()
	{
		auto *opToken = expect_read(TokenType::TK_OPERATOR);

		Operator op = getOperator(opToken->value);

		auto value = parseTerm();
		if (!value)
			return nullptr;

		value = std::make_unique<UnaryOperationAst>(getCurrentLocation(), std::move(value), op);

		return std::move(value);
	}

	std::unique_ptr<AstNode> Parser::parseParenthesis()
	{
		read();
		auto expr = parseExpression();

		if (expr == nullptr)
			return nullptr;

		this->expect_read(TokenType::TK_CLOSE_PARENTHESIS);

		return expr;
	}

	std::unique_ptr<AstNode> Parser::parseInteger()
	{
		auto token = expect_read(TokenType::TK_INTEGER);
		long value = atoi(token->value.c_str());

		auto node = std::make_unique<IntegerAst>(getCurrentLocation(), value);

		return std::move(node);
	}

	std::unique_ptr<AstNode> Parser::parseFloat()
	{
		auto token = expect_read(TokenType::TK_FLOAT);
		double value = atof(token->value.c_str());

		auto node = std::make_unique<FloatAst>(getCurrentLocation(), value);

		return std::move(node);
	}

	std::unique_ptr<AstNode> Parser::parseIdentifier()
	{
		std::unique_ptr<AstNode> result = nullptr;
		std::string identifier = expect_read(TokenType::TK_IDENTIFIER)->value;

		if (match_read(TokenType::TK_OPEN_PARENTHESIS))
		{
			// function call
			std::vector<std::unique_ptr<AstNode>> arguments;

			if (peek()->type != TokenType::TK_CLOSE_PARENTHESIS)
			{
				while (true)
				{
					auto arg = parseExpression();

					if (!arg)
						return nullptr;

					arguments.push_back(std::move(arg));

					if (peek()->type == TokenType::TK_CLOSE_PARENTHESIS)
						break;

					else if (peek()->type != TokenType::TK_COMMA)
					{
						state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, peek()->value));
						return nullptr;
					}
					read();
				}
			}
			read();

			result = std::make_unique<FunctionCallAst>(getCurrentLocation(), identifier, std::move(arguments));
		}
		else if (match_read(TokenType::TK_DOT))
		{
			std::unique_ptr<AstNode> nextIdentifier = nullptr;

			if (match(TokenType::TK_IDENTIFIER))
				nextIdentifier = std::move(parseIdentifier());
			else
				state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, read()->value));

			result = std::make_unique<MemberAccessAst>(getCurrentLocation(), identifier, std::move(nextIdentifier));
		}
		else
		{
			result = std::make_unique<VariableAst>(getCurrentLocation(), identifier);
		}

		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseString()
	{
		std::unique_ptr<AstNode> result = nullptr;
		std::string value = expect_read(TokenType::TK_STRING)->value;

		result = std::make_unique<StringAst>(getCurrentLocation(), value);

		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseTrue()
	{
		std::unique_ptr<AstNode> result = nullptr;
		std::string value = expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_TRUE))->value;

		result = std::make_unique<TrueAst>(getCurrentLocation());

		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseFalse()
	{
		std::unique_ptr<AstNode> result = nullptr;
		std::string value = expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_FALSE))->value;

		result = std::make_unique<FalseAst>(getCurrentLocation());

		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseNull()
	{
		std::unique_ptr<AstNode> result = nullptr;
		std::string value = expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_NULL))->value;

		result = std::make_unique<NullAst>(getCurrentLocation());

		return std::move(result);
	}

	std::unique_ptr<AstNode> Parser::parseTerm()
	{
		std::unique_ptr<AstNode> term = nullptr;

		if (!peek())
		{
			read();
			return nullptr;
		}

		if (match(TokenType::TK_OPEN_PARENTHESIS))
			term = std::move(parseParenthesis());
		else if (match(TokenType::TK_INTEGER))
			term = std::move(parseInteger());
		else if (match(TokenType::TK_FLOAT))
			term = std::move(parseFloat());
		else if (match(TokenType::TK_IDENTIFIER))
			term = std::move(parseIdentifier());
		else if (match(TokenType::TK_STRING))
			term = std::move(parseString());
		else if (match(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_TRUE)))
			term = std::move(parseTrue());
		else if (match(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_FALSE)))
			term = std::move(parseFalse());
		else if (match(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_NULL)))
			term = std::move(parseNull());
		else if (match(TokenType::TK_OPERATOR))
		{
			bool isUnary = false;

			if ((!peek(-1)) ||
				(peek(-1)->type == TokenType::TK_OPEN_PARENTHESIS) ||
				(peek(-1)->type == TokenType::TK_OPERATOR))
			{
				term = std::move(parseUnaryOperation());
			}
		}
		else
			state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, read()->value));

		return term;
	}

	std::unique_ptr<AstNode> Parser::parseExpression(bool shouldClearStack)
	{
		auto term = parseTerm();
		if (!term)
			return nullptr;

		if (match(TokenType::TK_OPERATOR))
		{
			// parse binary expression
			auto binOp = std::move(parseBinaryOperation(0, std::move(term)));
			if (!binOp)
				return nullptr;
			term = std::move(binOp);
		}

		auto expr = std::make_unique<ExpressionAst>(getCurrentLocation(), std::move(term), shouldClearStack);
		return std::move(expr);
	}

	std::unique_ptr<AstNode> Parser::parseBlock()
	{
		expect_read(TokenType::TK_OPEN_BRACE);

		auto block = std::make_unique<BlockAst>(getCurrentLocation());

		while (peek() && !match_read(TokenType::TK_CLOSE_BRACE))
		{
			block->addChild(parseStatement());
		}

		return std::move(block);
	}

	std::unique_ptr<AstNode> Parser::parseFunctionDefinition()
	{
		expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_FUNCTION));
		auto identifier = expect_read(TokenType::TK_IDENTIFIER)->value;
		expect_read(TokenType::TK_OPEN_PARENTHESIS);

		// function call
		std::vector<std::string> arguments;

		if (peek()->type != TokenType::TK_CLOSE_PARENTHESIS)
		{
			while (true)
			{
				auto argToken = expect_read(TokenType::TK_IDENTIFIER);
				auto arg = argToken->value;

				if (!argToken)
					return nullptr;

				arguments.push_back(arg);

				if (peek()->type == TokenType::TK_CLOSE_PARENTHESIS)
					break;

				else if (peek()->type != TokenType::TK_COMMA)
				{
					state.errors.push_back(Error(ErrorType::UNEXPECTED_TOKEN, getCurrentLocation(), state.filepath, peek()->value));
					return nullptr;
				}
				read();
			}
		}
		read();

		std::unique_ptr<AstNode> block = nullptr;
		if (peek() && peek()->type == TokenType::TK_OPEN_BRACE)
		{
			block = std::move(parseStatement()); // read the block
		}
		else
			state.errors.push_back(Error(ErrorType::EXPECTED_TOKEN, getCurrentLocation(), state.filepath, "{"));

		auto functionDefinitionAst = std::make_unique<FunctionDefinitionAst>(getCurrentLocation(), identifier, arguments, std::move(block));
		return std::move(functionDefinitionAst);
	}

	std::unique_ptr<AstNode> Parser::parseIfStatement()
	{
		expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_IF));
		expect_read(TokenType::TK_OPEN_PARENTHESIS);

		auto condition = parseExpression();

		expect_read(TokenType::TK_CLOSE_PARENTHESIS);

		std::unique_ptr<AstNode> block = nullptr;

		if (peek() && peek()->type == TokenType::TK_OPEN_BRACE)
			block = std::move(parseStatement()); // read the block
		else
		{
			auto blockAst = std::make_unique<BlockAst>(getCurrentLocation());
			blockAst->children.push_back(std::move(parseStatement()));
			block = std::move(blockAst);
		}

		std::unique_ptr<AstNode> elseBlock = nullptr;
		if (match_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_ELSE)))
		{
			if (peek() && peek()->type == TokenType::TK_OPEN_BRACE)
				elseBlock = std::move(parseStatement()); // read the block
			else
			{
				auto blockAst = std::make_unique<BlockAst>(getCurrentLocation());
				blockAst->children.push_back(std::move(parseStatement()));
				elseBlock = std::move(blockAst);
			}
		}

		auto ifStatementAst = std::make_unique<IfStatementAst>(getCurrentLocation(),
			std::move(condition),
			std::move(block),
			std::move(elseBlock));

		return std::move(ifStatementAst);
	}

	std::unique_ptr<AstNode> Parser::parseReturnStatement()
	{
		expect_read(TokenType::TK_KEYWORD, getKeywordStr(Keyword::KW_RETURN));
		auto expr = parseExpression();

		auto returnStatementAst = std::make_unique<ReturnStatementAst>(getCurrentLocation(),
			std::move(expr));

		return std::move(returnStatementAst);
	}

	std::unique_ptr<AstNode> Parser::parseForLoop()
	{
		return nullptr;
	}
}
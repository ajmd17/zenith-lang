#include "parser.h"

#include <iostream>
#include <stack>
#include <algorithm>
#include <functional>

#include "keywords.h"
#include "operators.h"

namespace zenith
{
	namespace compiler
	{
		Parser::Parser(std::vector<Token> tokens, LexerState lexerState)
		{
			state.tokens = tokens;
			state.errors = lexerState.errors;
			this->filepath = lexerState.location.file;
		}

		std::unique_ptr<ModuleAst> Parser::parse()
		{
			std::unique_ptr<ModuleAst> unit = nullptr;
			myModuleName = "__anon_module__";

			if (!match_read(TK_KEYWORD, getKeywordStr(KW_MODULE)))
				state.errors.push_back({ EXPECTED_MODULE_DECLARATION, location() });
			else
			{
				Token *ident = expect_read(TK_IDENTIFIER);
				if (ident)
				{
					myModuleName = ident->value;

					unit = std::make_unique<ModuleAst>(location(), myModuleName);
					moduleAst = unit.get();

					while (state.position < state.tokens.size())
						unit->addChild(parseStatement());

					return unit;
				}
			}

			// parse error occured
			unit = std::make_unique<ModuleAst>(location(), myModuleName);
			return unit;
		}

		SourceLocation Parser::location()
		{
			if (!state.tokens.empty())
			{
				Token *token = peek();
				return (token ? token->location : peek(-1)->location);
			}
			else return{ 0, 0, filepath };
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
			return (peek(n) && peek(n)->type == type);
		}

		bool Parser::match(TokenType type1, TokenType type2)
		{
			return (match(type1) && match(type2, 1));
		}

		bool Parser::match(TokenType type, const std::string &str)
		{
			return (match(type) && peek()->value == str);
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

				if (badToken)
				{
					switch (type)
					{
					case TK_IDENTIFIER:
						state.errors.push_back({ EXPECTED_IDENTIFIER, badToken->location });
						break;
					case TK_OPEN_PARENTHESIS:
					case TK_CLOSE_PARENTHESIS:
					case TK_OPEN_BRACE:
					case TK_CLOSE_BRACE:
					case TK_OPEN_BRACKET:
					case TK_CLOSE_BRACKET:
					case TK_SEMICOLON:
					case TK_COLON:
					case TK_COMMA:
						state.errors.push_back({ EXPECTED_TOKEN, badToken->location, Token::asString(type) });
						break;
					default:
						state.errors.push_back({ UNEXPECTED_TOKEN, badToken->location, badToken->value });
						break;
					}
				}
				else
					state.errors.push_back({ UNEXPECTED_END_OF_FILE, location() });
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
					state.errors.push_back({ EXPECTED_TOKEN, badToken->location, str });
				else
					state.errors.push_back({ UNEXPECTED_END_OF_FILE, location() });
			}
			return token;
		}

		int Parser::getOpPrecedence()
		{
			auto *current = peek();
			if (!(current && current->type == TK_OPERATOR))
				return -1;

			Operator op = getOperator(current->value);
			return getPrecedence(op);
		}

		std::unique_ptr<AstNode> Parser::parseImports()
		{
			auto token = expect_read(TK_KEYWORD, getKeywordStr(KW_IMPORT));

			bool isBlock = match_read(TK_OPEN_BRACE);
			if (!isBlock)
				return parseImport();

			std::vector<std::unique_ptr<AstNode>> imports;

			while (peek() != nullptr)
			{
				imports.push_back(parseImport());

				if (match_read(TK_CLOSE_BRACE) || !expect_read(TK_COMMA))
					break;
			}

			auto importsAst = std::make_unique<ImportsAst>(location(), moduleAst, std::move(imports));
			return std::move(importsAst);
		}

		std::unique_ptr<AstNode> Parser::parseImport()
		{
			const std::string localPath =
				filepath.substr(0, filepath.find_last_of("/\\")) + "/";

			Token *tk = nullptr;

			bool isModuleImport = true;
			std::string value;

			if (match_read(TK_STRING, tk))
			{
				isModuleImport = false;
				value = tk->value;
			}
			else if (match_read(TK_IDENTIFIER, tk))
			{
				isModuleImport = true;
				value = tk->value;
			}
			else
				state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

			auto result = std::make_unique<ImportAst>(location(), moduleAst, value, localPath, isModuleImport);
			return std::move(result);
		}

		std::unique_ptr<AstNode> Parser::parseStatement()
		{
			if (match(TK_KEYWORD))
			{
				const std::string &val = peek()->value;

				if (val == getKeywordStr(KW_VAR))
					return parseVarDeclaration();
				else if (val == getKeywordStr(KW_CLASS))
					return parseClass();
				/*else if (val == getKeyword(KW_ENUM))
				return parseEnum();*/
				else if (val == getKeywordStr(KW_IMPORT))
					return parseImports();
				else if (val == getKeywordStr(KW_FUNCTION))
					return parseFunctionDefinition();
				else if (val == getKeywordStr(KW_IF))
					return parseIfStatement();
				else if (val == getKeywordStr(KW_RETURN))
					return parseReturnStatement();
				else if (val == getKeywordStr(KW_FOR))
					return parseForLoop();
				/*else if (val == getKeyword(KW_FOREACH))
				return parseForEach();
				else if (val == getKeyword(KW_WHILE))
				return parseWhile();
				else if (val == getKeyword(KW_BREAK))
				return parseBreak();
				else if (val == getKeyword(KW_CONTINUE))
				return parseContinue();
				else if (val == getKeyword(KW_DO))
				return parseDoWhile();
				else if (val == getKeyword(KW_IMPORT))
				return parseImport();
				else if (val == getKeyword(KW_THROW))
				return parseThrow();
				else if (val == getKeyword(KW_TRY))
				return parseTry();
				else if (val == getKeyword(KW_SUPER))
				return parseSuper();*/
			}
			else if (match(TK_OPEN_BRACE))
				return parseBlock();
			else if (match_read(TK_SEMICOLON))
				return std::move(std::make_unique<StatementAst>(location(), moduleAst));

			// finally try parsing expression
			auto node = parseExpression(true);
			if (node == nullptr)
				state.errors.push_back({ ILLEGAL_EXPRESSION, location() });

			return std::move(node);
		}

		std::unique_ptr<AstNode> Parser::parseVarDeclaration()
		{
			auto token = expect_read(TK_KEYWORD, getKeywordStr(KW_VAR));
			auto identifier = peek()->value;

			lastVariable = identifier;

			std::unique_ptr<AstNode> assignment = nullptr;
			if (peek(1)->type == TK_OPERATOR)
			{
				if (peek(1)->value != getOperatorStr(Operator::OP_ASSIGN))
				{
					state.errors.push_back({ UNEXPECTED_TOKEN, location(), peek(1)->value });
					return nullptr;
				}
				else
					assignment = parseExpression();
			}
			else
			{
				// no expression following, so eat the identifier
				read();
			}

			lastVariable.clear();

			auto varDeclAst = std::make_unique<VariableDeclarationAst>(location(),
				moduleAst,
				identifier,
				std::move(assignment));

			return std::move(varDeclAst);
		}

		std::unique_ptr<AstNode> Parser::parseBinaryOperation(int exprPrec, std::unique_ptr<AstNode> left)
		{
			while (true)
			{
				int tokenPrec = getOpPrecedence();

				if (tokenPrec < exprPrec)
					return left;

				auto *opToken = expect_read(TK_OPERATOR);

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

				left = std::make_unique<BinaryOperationAst>(location(),
					moduleAst,
					std::move(left),
					std::move(right),
					op);
			}
			return nullptr;
		}

		std::unique_ptr<AstNode> Parser::parseUnaryOperation()
		{
			auto *opToken = expect_read(TK_OPERATOR);

			Operator op = getOperator(opToken->value);

			auto value = parseTerm();
			if (!value)
				return nullptr;

			value = std::make_unique<UnaryOperationAst>(location(),
				moduleAst,
				std::move(value),
				op);
			return std::move(value);
		}

		std::unique_ptr<AstNode> Parser::parseClass()
		{
			auto token = expect_read(TK_KEYWORD, getKeywordStr(KW_CLASS));
			auto ident = expect_read(TK_IDENTIFIER);
			std::vector<std::unique_ptr<AstNode>> dataMembers;

			expect_read(TK_OPEN_BRACE);
			while (!match_read(TK_CLOSE_BRACE))
			{
				dataMembers.push_back(std::move(parseStatement()));
			}

			auto classAst = std::make_unique<ClassAst>(location(),
				moduleAst,
				ident->value,
				std::move(dataMembers));
			return std::move(classAst);
		}

		std::unique_ptr<AstNode> Parser::parseParenthesis()
		{
			read();
			auto expr = parseExpression();

			if (expr == nullptr)
				return nullptr;

			this->expect_read(TK_CLOSE_PARENTHESIS);
			return expr;
		}

		std::unique_ptr<AstNode> Parser::parseInteger()
		{
			auto token = expect_read(TK_INTEGER);
			long value = atoi(token->value.c_str());

			auto node = std::make_unique<IntegerAst>(location(), moduleAst, value);
			return std::move(node);
		}

		std::unique_ptr<AstNode> Parser::parseFloat()
		{
			auto token = expect_read(TK_FLOAT);
			double value = atof(token->value.c_str());

			auto node = std::make_unique<FloatAst>(location(), moduleAst, value);
			return std::move(node);
		}

		std::unique_ptr<AstNode> Parser::parseIdentifier()
		{
			std::unique_ptr<AstNode> result = nullptr;
			std::string identifier = expect_read(TK_IDENTIFIER)->value;

			if (match_read(TK_OPEN_PARENTHESIS))
			{
				// function call
				std::vector<std::unique_ptr<AstNode>> arguments;

				if (peek()->type != TK_CLOSE_PARENTHESIS)
				{
					while (true)
					{
						auto arg = parseExpression();

						if (!arg)
							return nullptr;

						arguments.push_back(std::move(arg));

						if (peek()->type == TK_CLOSE_PARENTHESIS)
							break;

						else if (peek()->type != TK_COMMA)
						{
							state.errors.push_back({ UNEXPECTED_TOKEN, location(), peek()->value });
							return nullptr;
						}
						read();
					}
				}
				read();

				result = std::make_unique<FunctionCallAst>(location(), 
					moduleAst, 
					identifier, 
					std::move(arguments));
			}
			else
				result = std::make_unique<VariableAst>(location(), moduleAst, identifier);

			// check for member access (something.whatever) after
			if (match_read(TK_DOT))
			{
				std::unique_ptr<AstNode> nextIdentifier = nullptr;

				if (match(TK_IDENTIFIER))
					nextIdentifier = std::move(parseIdentifier());
				else
					state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

				result = std::make_unique<MemberAccessAst>(location(),
					moduleAst,
					identifier,
					std::move(result),
					std::move(nextIdentifier));
			}

			return std::move(result);
		}

		std::unique_ptr<AstNode> Parser::parseString()
		{
			std::unique_ptr<AstNode> result = nullptr;
			std::string value = expect_read(TK_STRING)->value;

			result = std::make_unique<StringAst>(location(), moduleAst, value);
			return std::move(result);
		}

		std::unique_ptr<AstNode> Parser::parseTrue()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_TRUE));
			return std::make_unique<TrueAst>(location(), moduleAst);
		}

		std::unique_ptr<AstNode> Parser::parseFalse()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_FALSE));
			return std::make_unique<FalseAst>(location(), moduleAst);
		}

		std::unique_ptr<AstNode> Parser::parseNull()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_NULL));
			return std::make_unique<NullAst>(location(), moduleAst);
		}

		std::unique_ptr<AstNode> Parser::parseSelf()
		{
			std::string identifier = expect_read(TK_KEYWORD, getKeywordStr(KW_SELF))->value;
			std::unique_ptr<AstNode> result = std::make_unique<SelfAst>(location(), moduleAst);

			if (match_read(TK_DOT))
			{
				std::unique_ptr<AstNode> nextIdentifier = nullptr;
				
				std::string leftStr = peek()->value;
				if (match(TK_IDENTIFIER))
					nextIdentifier = std::move(parseIdentifier());
				else
					state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

				result = std::make_unique<MemberAccessAst>(location(),
					moduleAst,
					leftStr,
					std::move(result),
					std::move(nextIdentifier));
			}

			return std::move(result);
		}

		std::unique_ptr<AstNode> Parser::parseNew(const std::string &identifier)
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_NEW));

			std::unique_ptr<AstNode> ident;
			if (match(TK_IDENTIFIER))
				ident = std::move(parseIdentifier());
			else
				state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

			auto newAst = std::make_unique<NewAst>(location(), 
				moduleAst, 
				identifier,
				std::move(ident));
			return std::move(newAst);
		}

		std::unique_ptr<AstNode> Parser::parseTerm()
		{
			std::unique_ptr<AstNode> term = nullptr;

			if (!peek())
			{
				read();
				return nullptr;
			}

			if (match(TK_OPEN_PARENTHESIS))
				term = std::move(parseParenthesis());
			else if (match(TK_INTEGER))
				term = std::move(parseInteger());
			else if (match(TK_FLOAT))
				term = std::move(parseFloat());
			else if (match(TK_IDENTIFIER))
				term = std::move(parseIdentifier());
			else if (match(TK_STRING))
				term = std::move(parseString());
			else if (match(TK_KEYWORD, getKeywordStr(KW_TRUE)))
				term = std::move(parseTrue());
			else if (match(TK_KEYWORD, getKeywordStr(KW_FALSE)))
				term = std::move(parseFalse());
			else if (match(TK_KEYWORD, getKeywordStr(KW_NULL)))
				term = std::move(parseNull());
			else if (match(TK_KEYWORD, getKeywordStr(KW_SELF)))
				term = std::move(parseSelf());
			else if (match(TK_KEYWORD, getKeywordStr(KW_NEW)))
				term = std::move(parseNew(lastVariable));
			else if (match(TK_OPERATOR))
			{
				bool isUnary = false;

				if ((!peek(-1)) ||
					(peek(-1)->type == TK_OPEN_PARENTHESIS) ||
					(peek(-1)->type == TK_OPERATOR))
				{
					term = std::move(parseUnaryOperation());
				}
			}
			else
				state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

			return term;
		}

		std::unique_ptr<AstNode> Parser::parseExpression(bool shouldClearStack)
		{
			auto term = parseTerm();
			if (!term)
				return nullptr;

			if (match(TK_OPERATOR))
			{
				// parse binary expression
				auto binOp = std::move(parseBinaryOperation(0, std::move(term)));
				if (!binOp)
					return nullptr;
				term = std::move(binOp);
			}

			auto expr = std::make_unique<ExpressionAst>(location(), 
				moduleAst, 
				std::move(term), 
				shouldClearStack);
			return std::move(expr);
		}

		std::unique_ptr<AstNode> Parser::parseBlock()
		{
			expect_read(TK_OPEN_BRACE);
			auto block = std::make_unique<BlockAst>(location(), moduleAst);
			while (peek() && !match_read(TK_CLOSE_BRACE))
				block->addChild(parseStatement());

			return std::move(block);
		}

		std::unique_ptr<AstNode> Parser::parseFunctionDefinition()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_FUNCTION));
			auto identifier = expect_read(TK_IDENTIFIER)->value;
			expect_read(TK_OPEN_PARENTHESIS);

			std::vector<std::string> arguments;
			if (peek()->type != TK_CLOSE_PARENTHESIS)
			{
				while (true)
				{
					auto argToken = expect_read(TK_IDENTIFIER);
					auto arg = argToken->value;

					if (!argToken)
						return nullptr;

					arguments.push_back(arg);

					if (peek()->type == TK_CLOSE_PARENTHESIS)
						break;

					else if (peek()->type != TK_COMMA)
					{
						state.errors.push_back({ UNEXPECTED_TOKEN, location(), peek()->value });
						return nullptr;
					}
					read();
				}
			}
			read();

			std::unique_ptr<AstNode> block = nullptr;
			if (peek() && peek()->type == TK_OPEN_BRACE)
				block = std::move(parseStatement()); // read the block
			else
				state.errors.push_back({ UNEXPECTED_TOKEN, location(), read()->value });

			auto functionDefinitionAst = std::make_unique<FunctionDefinitionAst>(location(),
				moduleAst,
				identifier,
				arguments,
				std::move(block));
			return std::move(functionDefinitionAst);
		}

		std::unique_ptr<AstNode> Parser::parseIfStatement()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_IF));
			expect_read(TK_OPEN_PARENTHESIS);
			auto condition = parseExpression();
			expect_read(TK_CLOSE_PARENTHESIS);

			std::unique_ptr<AstNode> block = nullptr;
			if (peek() && peek()->type == TK_OPEN_BRACE)
				block = std::move(parseStatement());
			else
			{
				auto blockAst = std::make_unique<BlockAst>(location(), moduleAst);
				blockAst->children.push_back(std::move(parseStatement()));
				block = std::move(blockAst);
			}

			std::unique_ptr<AstNode> elseBlock = nullptr;
			if (match_read(TK_KEYWORD, getKeywordStr(KW_ELSE)))
			{
				if (peek() && peek()->type == TK_OPEN_BRACE)
					elseBlock = std::move(parseStatement()); // read the block
				else
				{
					auto blockAst = std::make_unique<BlockAst>(location(), moduleAst);
					blockAst->children.push_back(std::move(parseStatement()));
					elseBlock = std::move(blockAst);
				}
			}

			auto ifStatementAst = std::make_unique<IfStatementAst>(location(),
				moduleAst,
				std::move(condition),
				std::move(block),
				std::move(elseBlock));
			return std::move(ifStatementAst);
		}

		std::unique_ptr<AstNode> Parser::parseReturnStatement()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_RETURN));
			auto expr = parseExpression();

			auto returnStatementAst = std::make_unique<ReturnStatementAst>(location(), 
				moduleAst, 
				std::move(expr));
			return std::move(returnStatementAst);
		}

		std::unique_ptr<AstNode> Parser::parseForLoop()
		{
			expect_read(TK_KEYWORD, getKeywordStr(KW_FOR));
			expect_read(TK_OPEN_PARENTHESIS);

			// read initializer
			std::unique_ptr<AstNode> init_expr = nullptr;
			if (!match_read(TK_SEMICOLON))
			{
				init_expr = parseStatement();
				expect_read(TK_SEMICOLON);
			}

			std::unique_ptr<AstNode> cond_expr = nullptr;
			if (match_read(TK_SEMICOLON))
				cond_expr = std::make_unique<TrueAst>(location(), moduleAst);
			else
			{
				cond_expr = parseExpression();
				expect_read(TK_SEMICOLON);
			}

			std::unique_ptr<AstNode> inc_expr = nullptr;
			if (!match_read(TK_CLOSE_PARENTHESIS))
			{
				inc_expr = parseExpression();
				expect_read(TK_CLOSE_PARENTHESIS);
			}

			std::unique_ptr<AstNode> block = nullptr;
			if (peek() && peek()->type == TK_OPEN_BRACE)
				block = std::move(parseStatement()); // read the block
			else
			{
				auto blockAst = std::make_unique<BlockAst>(location(), moduleAst);
				blockAst->children.push_back(std::move(parseStatement()));
				block = std::move(blockAst);
			}

			auto forLoopAst = std::make_unique<ForLoopAst>(location(),
				moduleAst,
				std::move(init_expr),
				std::move(cond_expr),
				std::move(inc_expr),
				std::move(block));
			return std::move(forLoopAst);
		}
	}
}
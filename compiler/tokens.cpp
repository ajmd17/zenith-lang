#include "tokens.h"

namespace zenith
{
	std::map<TokenType, std::string> Token::tokenStrings =
	{
		{ TK_UNDEFINED, "undefined" },
		{ TK_IDENTIFIER, "identifier" },
		{ TK_STRING, "string" },
		{ TK_INTEGER, "integer" },
		{ TK_FLOAT, "float" },
		{ TK_KEYWORD, "keyword" },
		{ TK_OPERATOR, "operator" },
		{ TK_OPEN_PARENTHESIS, "(" },
		{ TK_CLOSE_PARENTHESIS, ")" },
		{ TK_OPEN_BRACE, "{" },
		{ TK_CLOSE_BRACE, "}" },
		{ TK_OPEN_BRACKET, "[" },
		{ TK_CLOSE_BRACKET, "]" },
		{ TK_SEMICOLON, ";" },
		{ TK_COLON, ":" },
		{ TK_COMMA, "," },
		{ TK_DOT, "." }
	};

	std::string Token::asString(TokenType type)
	{
		return tokenStrings.at(type);
	}
}
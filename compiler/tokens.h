#ifndef __ZENITH_COMPILER_TOKENS_H__
#define __ZENITH_COMPILER_TOKENS_H__

#include <string>
#include <map>

#include "src_location.h"

namespace zenith
{
	namespace compiler
	{
		enum TokenType
		{
			TK_UNDEFINED = 0,
			TK_IDENTIFIER,
			TK_STRING,
			TK_INTEGER,
			TK_FLOAT,
			TK_KEYWORD,
			TK_OPERATOR,
			TK_OPEN_PARENTHESIS,
			TK_CLOSE_PARENTHESIS,
			TK_OPEN_BRACE,
			TK_CLOSE_BRACE,
			TK_OPEN_BRACKET,
			TK_CLOSE_BRACKET,
			TK_SEMICOLON,
			TK_COLON,
			TK_COMMA,
			TK_DOT
		};

		struct Token
		{
		public:
			zenith::TokenType type;
			std::string value;
			SourceLocation location;

		private:
			static std::map<zenith::TokenType, std::string> tokenStrings;

		public:
			static std::string asString(zenith::TokenType type);
		};
	}
}

#endif
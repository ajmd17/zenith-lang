#ifndef __LEXER_H__
#define __LEXER_H__

#include <string>
#include <vector>
#include <ctype.h>
#include <cstdint>

#include "state.h"

namespace zenith
{
	class Lexer
	{
	public:
		LexerState state;

		Lexer(const std::string &source, const std::string &filepath = "");
		std::vector<Token> scan();

	private:
		Token nextToken();
		Token readNumber();
		Token readFloat(std::string &str);
		Token readString();
		Token readMultilineString();
		Token readIdentifier();
		Token readOperator();

		void readLineComment();
		void readBlockComment();

		char readChar();
		char peekChar(int n = 0);
		char parseEscapeCode();
		bool skipWhitespace();
	};
}

#endif
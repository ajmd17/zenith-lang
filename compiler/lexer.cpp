#include "lexer.h"

#include "operators.h"
#include "keywords.h"

namespace zenith
{
	namespace compiler
	{
		Lexer::Lexer(const std::string &source, const std::string &filepath)
		{
			this->state.source = source;
			state.location.file = filepath;
			state.position = 0;
			state.sourceLen = source.length();
			state.location = SourceLocation(0, 0, filepath);
		}

		std::vector<Token> Lexer::scan()
		{
			std::vector<Token> tokens;
			skipWhitespace();
			while (peekChar() != '\0')
			{
				Token nextToken = this->nextToken();
				if (nextToken.type != TokenType::TK_UNDEFINED)
					tokens.push_back(nextToken);

				skipWhitespace();
			}

			return tokens;
		}

		Token Lexer::nextToken()
		{
			char ch = peekChar();

			if (ch == '\'' || ch == '"')
				return readString();
			else if (isdigit(ch))
				return readNumber();
			else if (ch == '_' || isalpha(ch))
				return readIdentifier();
			else if (ch == '+' ||
				ch == '-' ||
				ch == '*' ||
				ch == '/' ||
				ch == '%' ||
				ch == '=' ||
				ch == '!' ||
				ch == '<' ||
				ch == '>' ||
				ch == '&' ||
				ch == '|' ||
				ch == '^' ||
				ch == '?' ||
				ch == '.')
				return readOperator();
			else if (ch == '{')
			{
				readChar();
				return{ TokenType::TK_OPEN_BRACE, "{", state.location };
			}
			else if (ch == '}')
			{
				readChar();
				return{ TokenType::TK_CLOSE_BRACE, "}", state.location };
			}
			else if (ch == '[')
			{
				readChar();
				return{ TokenType::TK_OPEN_BRACKET, "[", state.location };
			}
			else if (ch == ']')
			{
				readChar();
				return{ TokenType::TK_CLOSE_BRACKET, "]", state.location };
			}
			else if (ch == '(')
			{
				readChar();
				return{ TokenType::TK_OPEN_PARENTHESIS, "(", state.location };
			}
			else if (ch == ')')
			{
				readChar();
				return{ TokenType::TK_CLOSE_PARENTHESIS, ")", state.location };
			}
			else if (ch == ';')
			{
				readChar();
				return{ TokenType::TK_SEMICOLON, ";", state.location };
			}
			else if (ch == ':')
			{
				readChar();
				return{ TokenType::TK_COLON, ":", state.location };
			}
			else if (ch == ',')
			{
				readChar();
				return{ TokenType::TK_COMMA, ",", state.location };
			}
			else
			{
				state.errors.push_back({ ErrorType::UNEXPECTED_TOKEN, state.location, readChar() });

				return{};
			}
		}

		Token Lexer::readNumber()
		{
			std::string str = "";
			char ch = peekChar();

			do
			{
				if (ch == '.')
					return readFloat(str);

				str += readChar();
				ch = peekChar();
			} while (isdigit(ch) || ch == '.');

			return{ TokenType::TK_INTEGER, str, state.location };
		}

		Token Lexer::readFloat(std::string &str)
		{
			readChar();
			str += ".";

			char ch = peekChar();

			do
			{
				str += readChar();
				ch = peekChar();
			} while (isdigit(ch));

			return{ TokenType::TK_FLOAT, str, state.location };
		}

		Token Lexer::readString()
		{
			int delimiter = peekChar();

			if (peekChar(1) == delimiter &&
				peekChar(2) == delimiter)
			{
				return readMultilineString();
			}
			else
			{
				readChar(); // eat the delimiter

				std::string str;

				char ch = peekChar();
				while (ch != delimiter)
				{
					if (ch == '\\')
					{
						readChar();
						str += parseEscapeCode();
					}
					else if (ch == '\0' || ch == '\n')
						break;
					else
						str += readChar();

					ch = peekChar();
				}

				if (readChar() != delimiter)
					state.errors.push_back({ ErrorType::UNTERMINATED_STRING_LITERAL, state.location });

				return{ TokenType::TK_STRING, str, state.location };
			}
		}

		Token Lexer::readMultilineString()
		{
			char delimiter = readChar();
			readChar();
			readChar();

			std::string str;

			char ch = peekChar();
			while (ch != delimiter)
			{
				if (ch == '\\')
				{
					readChar();
					str += parseEscapeCode();
				}
				else if (ch == '\0')
					break;
				else
					str += readChar();

				ch = peekChar();
			}

			if (!(readChar() == delimiter &&
				readChar() == delimiter &&
				readChar() == delimiter))
			{
				state.errors.push_back({ ErrorType::UNTERMINATED_STRING_LITERAL, state.location });
			}

			return{ TokenType::TK_STRING, str, state.location };
		}

		Token Lexer::readIdentifier()
		{
			std::string str;
			char ch = peekChar();

			do
			{
				str += readChar();
				ch = peekChar();
			} while ((isdigit(ch) || isalpha(ch)) || ch == '_');

			Keyword kw = getKeyword(str);
			if (kw == Keyword::KW_INVALID)
				return{ TokenType::TK_IDENTIFIER, str, state.location };
			else
				return{ TokenType::TK_KEYWORD, str, state.location };
		}

		Token Lexer::readOperator()
		{
			char ch = readChar();

			std::string nextTwoChars;
			nextTwoChars += ch;
			nextTwoChars += peekChar();

			Operator op = getOperator(nextTwoChars);
			if (op != Operator::OP_INVALID)
			{
				readChar();
				return{ TokenType::TK_OPERATOR, nextTwoChars, state.location };
			}
			else if (nextTwoChars == "/*")
			{
				readChar();
				readBlockComment();
				return{ TokenType::TK_UNDEFINED, nextTwoChars, state.location };
			}
			else if (nextTwoChars == "//")
			{
				readChar();
				readLineComment();
				return{ TokenType::TK_UNDEFINED, nextTwoChars, state.location };
			}

			std::string strOp;
			strOp += ch;
			switch (ch)
			{
			case '.':
				return{ TokenType::TK_DOT, strOp, state.location };
			default:
				return{ TokenType::TK_OPERATOR, strOp, state.location };
			}
		}

		void Lexer::readLineComment()
		{
			char ch;
			do
			{
				ch = readChar();
			} while (ch != '\0' && ch != '\n');
		}

		void Lexer::readBlockComment()
		{
			while (peekChar() != '\0')
			{
				char ch = readChar();

				if (ch == '*')
				{
					if (peekChar() == '/')
					{
						readChar();
						return;
					}
				}
			}

			state.errors.push_back({ ErrorType::UNEXPECTED_END_OF_FILE, state.location });
		}

		char Lexer::readChar()
		{
			if (state.position >= state.sourceLen)
				return '\0';

			if (state.source[state.position] == '\n')
			{
				state.location.line++;
				state.location.column = 0;
			}
			else
				state.location.column++;

			return state.source[state.position++];
		}

		char Lexer::peekChar(int n)
		{
			if ((state.position + n) >= state.sourceLen)
				return '\0';

			return state.source[state.position + n];
		}

		char Lexer::parseEscapeCode()
		{
			char ch = readChar();

			switch (ch)
			{
			case '"':
				return '"';
			case '\'':
				return '\'';
			case 'n':
				return '\n';
			case 'b':
				return '\b';
			case 'r':
				return '\r';
			case 'v':
				return '\v';
			case 't':
				return '\t';
			case 'f':
				return '\f';
			case '\\':
				return '\\';
			}

			state.errors.push_back({ ErrorType::UNRECOGNIZED_ESCAPE_SEQUENCE, state.location, ch });
			return '\0';
		}

		bool Lexer::skipWhitespace()
		{
			bool hadNewLine = false;
			while (isspace(peekChar()))
			{
				char ch = readChar();

				if (ch == '\n')
					hadNewLine = true;
			}

			return hadNewLine;
		}
	}
}
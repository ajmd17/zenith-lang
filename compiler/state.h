#ifndef __STATE_H__
#define __STATE_H__

#include <vector>
#include <string>

#include "errors.h"
#include "tokens.h"
#include "src_location.h"

namespace zenith
{
	struct ParserState
	{
		std::vector<Error> errors;
		std::vector<Token> tokens;

		int position = 0;

		std::string filepath;
	};

	struct LexerState
	{
		int position;
		int sourceLen;

		std::string source;
		std::string file;

		SourceLocation location;

		std::vector<Error> errors;

		std::string filepath;
	};
}

#endif
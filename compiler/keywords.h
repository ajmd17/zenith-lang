#ifndef __KEYWORDS_H__
#define __KEYWORDS_H__

#include <string>
#include <vector>
#include <algorithm>

namespace zenith
{
	enum Keyword
	{
		KW_INVALID = -1,
		KW_IF,
		KW_ELSE,
		KW_DO,
		KW_WHILE,
		KW_FOR,
		KW_FOREACH,
		KW_SWITCH,
		KW_CASE,
		KW_BREAK,
		KW_CONTINUE,
		KW_DEFAULT,
		KW_RETURN,
		KW_GOTO,
		KW_TRY,
		KW_CATCH,
		KW_THROW,
		KW_CLASS,
		KW_ENUM,
		KW_STRUCT,
		KW_VOID,
		KW_TRUE,
		KW_FALSE,
		KW_NULL,
		KW_VAR,
		KW_STATIC,
		KW_PUBLIC,
		KW_PRIVATE,
		KW_PROTECTED,
		KW_CONST,
		KW_FUNCTION,
		KW_SUPER,
		KW_MODULE,
		KW_PACKAGE,
		KW_TYPEOF,
		KW_IS,
		KW_AS,
		KW_CAST,
		KW_IMPORT,
		KW_THIS,
		KW_NEW,
		KW_DELETE
	};

	const std::map<std::string, Keyword> keywords = 
	{
		{ "if", KW_IF },
		{ "else", KW_ELSE },
		{ "do", KW_DO },
		{ "while", KW_WHILE }, 
		{ "for", KW_FOR },
		{ "foreach", KW_FOREACH },
		{ "switch", KW_SWITCH },
		{ "case", KW_CASE },
		{ "break", KW_BREAK },
		{ "continue", KW_CONTINUE },
		{ "default", KW_DEFAULT },
		{ "return", KW_RETURN },
		{ "goto", KW_GOTO },
		{ "try", KW_TRY },
		{ "catch", KW_CATCH },
		{ "throw", KW_THROW },
		{ "class", KW_CLASS },
		{ "enum", KW_ENUM },
		{ "struct", KW_STRUCT },
		{ "void", KW_VOID },
		{ "true", KW_TRUE },
		{ "false", KW_FALSE },
		{ "null", KW_NULL },
		{ "var", KW_VAR },
		{ "static", KW_STATIC },
		{ "public", KW_PUBLIC },
		{ "private", KW_PRIVATE },
		{ "protected", KW_PROTECTED },
		{ "const", KW_CONST },
		{ "fn", KW_FUNCTION },
		{ "super", KW_SUPER },
		{ "module", KW_MODULE },
		{ "package", KW_PACKAGE },
		{ "typeof", KW_TYPEOF },
		{ "is", KW_IS },
		{ "as", KW_AS },
		{ "cast", KW_CAST },
		{ "import", KW_IMPORT },
		{ "this", KW_THIS },
		{ "new", KW_NEW },
		{ "delete", KW_DELETE }
	};

	inline const std::string &getKeywordStr(Keyword kw)
	{
		auto it = std::find_if(keywords.begin(), keywords.end(), 
			[&kw](const std::pair<std::string, Keyword> &item) {
					return item.second == kw;
				});
		
		if (it == keywords.end())
			throw std::out_of_range("Keyword has no string equivalent");
		
		return it->first;
	}

	inline const Keyword getKeyword(const std::string &str)
	{
		auto it = keywords.find(str);

		if (it == keywords.end())
			return Keyword::KW_INVALID;
		else
			return it->second;
	}
}

#endif
#include "errors.h"

#include <iostream>

namespace zenith
{
	namespace compiler
	{
		std::map<ErrorType, std::string> Error::errorMessages =
		{
			{ INTERNAL_ERROR, "Internal error" },
			{ ILLEGAL_SYNTAX, "Illegal syntax" },
			{ ILLEGAL_EXPRESSION, "Illegal expression" },
			{ ILLEGAL_OPERATOR, "Illegal operator '%'" },
			{ UNBALANCED_EXPRESSION, "Unbalanced expression" },
			{ UNEXPECTED_CHARACTER, "Unexpected character '%'" },
			{ UNEXPECTED_IDENTIFIER, "Unexpected identifier '%'" },
			{ UNEXPECTED_TOKEN, "Unexpected token '%'" },
			{ UNRECOGNIZED_ESCAPE_SEQUENCE, "Unrecognized escape sequence '%'" },
			{ UNTERMINATED_STRING_LITERAL, "Unterminated string literal" },
			{ ALREADY_DEFINED, "Identifier '%' already defined" },
			{ ARGUMENT_AFTER_KEYWORD_ARGS, "Argument not allowed after keyword arguments" },
			{ ARGUMENT_AFTER_VARIADIC_ARGS, "Argument not allowed after variadic arguments" },
			{ FUNCTION_NOT_FOUND, "Function '%' does not exist in this scope" },
			{ TOO_MANY_ARGS, "Too many arguments used for function '%'" },
			{ TOO_FEW_ARGS, "Too few arguments used for function '%'" },
			{ REDECLARED_IDENTIFIER, "Identifier '%' has already been declared in this scope"},
			{ UNDECLARED_IDENTIFIER, "Identifier '%' has not been declared"},
			{ EXPECTED_IDENTIFIER, "Expected an identifier" },
			{ AMBIGUOUS_IDENTIFIER, "Identifier '%' is ambiguous"},
			{ INVALID_CONSTRUCTOR, "Invalid constructor"},
			{ UNKNOWN_CLASS_TYPE, "Unknown class type, '%'"},
			{ EXPECTED_TOKEN, "Expected '%'" },
			{ UNEXPECTED_END_OF_FILE, "Unexpected end of file" },
			{ EXPECTED_MODULE_DECLARATION, "Expected 'module' declaration" },
			{ MODULE_NOT_FOUND, "Module '%' could not be found" },
			{ MODULE_ALREADY_DEFINED, "Module '%' has already been defined" },
			{ IMPORT_OUTSIDE_GLOBAL, "Import not allowed outside of global scope"},
			{ SELF_NOT_DEFINED, "'self' not allowed outside of a class" }
		};

		void Error::display()
		{
			std::cout <<
				"\tLn " <<
				(location.line + 1) <<
				" Col " <<
				(location.column + 1) <<

				": " << text << "\n";
		}

		bool Error::operator<(const Error &other)
		{
			return location.line < other.location.line;
		}
	}
}
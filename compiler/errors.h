#ifndef __ZENITH_COMPILER_ERRORS_H__
#define __ZENITH_COMPILER_ERRORS_H__

#include <string>
#include <map>
#include <sstream> 
#include <iostream>
#include "src_location.h"

namespace zenith
{
	namespace compiler
	{
		enum ErrorType
		{
			INTERNAL_ERROR,
			ILLEGAL_SYNTAX,
			ILLEGAL_EXPRESSION,
			ILLEGAL_OPERATOR,
			UNBALANCED_EXPRESSION,
			UNEXPECTED_CHARACTER,
			UNEXPECTED_IDENTIFIER,
			UNEXPECTED_TOKEN,
			UNRECOGNIZED_ESCAPE_SEQUENCE,
			UNTERMINATED_STRING_LITERAL,
			ALREADY_DEFINED,
			ARGUMENT_AFTER_KEYWORD_ARGS,
			ARGUMENT_AFTER_VARIADIC_ARGS,
			FUNCTION_NOT_FOUND,
			TOO_MANY_ARGS,
			TOO_FEW_ARGS,
			REDECLARED_IDENTIFIER,
			UNDECLARED_IDENTIFIER,
			EXPECTED_IDENTIFIER,
			AMBIGUOUS_IDENTIFIER,
			INVALID_CONSTRUCTOR,
			UNKNOWN_CLASS_TYPE,
			EXPECTED_TOKEN,
			UNEXPECTED_END_OF_FILE,
			EXPECTED_MODULE_DECLARATION,
			MODULE_NOT_FOUND,
			MODULE_ALREADY_DEFINED,
			IMPORT_OUTSIDE_GLOBAL,
			SELF_NOT_DEFINED
		};

		struct Error
		{
		public:
			ErrorType type;
			SourceLocation location;

			std::string text;
			std::string filename;

		private:
			void makeErrorMessage(const char *format) // base function
			{
				text += format;
			}

			template <typename T, typename ... Args>
			void makeErrorMessage(const char *format, T value, Args && ... args)
			{
				for (; *format != '\0'; format++)
				{
					if (*format == '%')
					{
						std::ostringstream sstream;
						sstream << value;
						text += sstream.str();

						makeErrorMessage(format + 1, args...);
						return;
					}
					text += *format;
				}
			}

			static std::map<ErrorType, std::string> errorMessages;

		public:
			template <typename ... Args>
			Error(ErrorType type, SourceLocation location, Args && ... args)
			{
				this->type = type;
				this->location = location;

				makeErrorMessage(errorMessages[type].c_str(), args...);
			}

			void display();
			bool operator<(const Error &other);
		};
	}
}

#endif
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <cstdio>
#include <iostream>
#include <string>

namespace zenith
{
	struct Exception
	{
		std::string message;

		void display()
		{
			printf("%s: %s", typeid(*this).name(), message.c_str());
			std::cin.get();
			abort();
		}
	};

	struct BinaryOperatorException 
		: public Exception
	{
		BinaryOperatorException(const std::string &typeA, const std::string &typeB)
			: Exception({"To conversion found between types '" + typeA + "' and '" + typeB + "'"})
		{
		}
	};

	struct UnaryOperatorException
		: public Exception
	{
		UnaryOperatorException(const std::string &type)
			: Exception({ "Invalid operation on type '" + type + "'" })
		{
		}
	};
}

#endif
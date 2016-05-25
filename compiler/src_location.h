#ifndef __ZENITH_COMPILER_SRC_LOCATION_H__
#define __ZENITH_COMPILER_SRC_LOCATION_H__

#include <string>

namespace zenith
{
	namespace compiler
	{
		struct SourceLocation
		{
			int line;
			int column;
			std::string file;

			SourceLocation()
			{
			}

			SourceLocation(const SourceLocation &other)
			{
				this->line = other.line;
				this->column = other.column;
				this->file = other.file;
			}

			SourceLocation(int line, int column, const std::string &file)
			{
				this->line = line;
				this->column = column;
				this->file = file;
			}
		};
	}
}

#endif
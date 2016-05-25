#ifndef __ZENITH_UTIL_LOGGER_H__
#define __ZENITH_UTIL_LOGGER_H__

#define DEBUG_PRINT 0

#include <cstdio>

namespace zenith 
{
	namespace util 
	{
		template <typename...Args>
		inline void debug_log(const char *format, Args &&... args)
		{
			#if DEBUG_PRINT
			printf(format, args...);
			printf("\n");
			#endif
		}
	}
}

#endif

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstdio>

#define DEBUG_PRINT 0

template <typename...Args>
inline void debug_log(const char *format, Args &&... args)
{
	#if DEBUG_PRINT
	printf(format, args...);
	printf("\n");
	#endif
}

#endif

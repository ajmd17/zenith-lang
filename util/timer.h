#ifndef __TIMER_H__
#define __TIMER_H__

#include <time.h>

namespace zenith
{
	class Timer 
	{
	private:
		unsigned long beginning;
	public:
		void start() 
		{
			beginning = clock();
		}

		double elapsedTime() const
		{
			return ((double)(clock() - beginning)) / CLOCKS_PER_SEC;
		}
	};
}

#endif
#ifndef __MODULE_H__
#define __MODULE_H__

#include <string>
#include <vector>

#include "frame.h"
#include "../enums.h"

namespace zenith
{
	class Module
	{
	private:
		std::map<int, StackFrame> frames;
		std::string _name;
		std::vector<unsigned long> fnPositionChain;
		std::map<int, unsigned long> savedPositions;

	public:
		Module(const std::string &name);
		~Module();

		void createFrame(int level);
		void leaveFrame(int level);
		StackFrame &getFrame(int level);

		void pushFunctionChain(unsigned long pos);
		unsigned long popFunctionChain();

		// map block id to saved position
		std::map<int, unsigned long> &getSavedPositions()
		{
			return savedPositions;
		}

		const std::string &name() const
		{
			return _name;
		}
	};
}

#endif
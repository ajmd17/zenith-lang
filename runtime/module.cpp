#include "module.h"

#include "exception.h"

namespace zenith 
{
	namespace runtime
	{
		Module::Module(const std::string &name)
		{
			_name = name;

			// create global stack frame
			createFrame(-1);
		}

		Module::~Module()
		{
			frames.clear();
		}

		void Module::createFrame(int level)
		{
			if (frames.find(level) != frames.end())
				Exception({ "Stack frame [" + std::to_string(level) + "] already exists" }).display();

			frames.insert({ level, StackFrame() });
		}

		void Module::leaveFrame(int level)
		{
			auto pos = frames.find(level);
			if (pos == frames.end())
				Exception({ "Stack frame [" + std::to_string(level) + "] does not exist" }).display();

			frames.erase(pos);
		}

		StackFrame &Module::getFrame(int level)
		{
			if (level < -1)
				Exception({ "Tried to access a frame below global" }).display();

			if (frames.find(level) == frames.end())
				Exception({ "Tried to access a frame that does not exist" }).display();

			return frames[level];
		}

		void Module::pushFunctionChain(unsigned long pos)
		{
			fnPositionChain.push_back(pos);
		}

		unsigned long Module::popFunctionChain()
		{
			unsigned long ret = fnPositionChain.back();
			fnPositionChain.pop_back();

			return ret;
		}
	}
}
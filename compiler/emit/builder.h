#ifndef __BUILDER_H__
#define __BUILDER_H__

#include "bytecode.h"

namespace zenith
{
	class BytecodeBuilder
	{
	private:
		BytecodeCommandList commandList;
	public:
		BytecodeBuilder() { }

		template <typename T, typename ... Args>
		typename std::enable_if<std::is_base_of<BytecodeCommand, T>::value, void>::type
			addCommand(Args... args)
		{
			auto tPtr = std::make_shared<T>(args...);
			commandList.push_back(tPtr);
		}

		BytecodeCommandList &getCommands() { return commandList; }
	};
}

#endif
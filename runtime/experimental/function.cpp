#include "function.h"

#include "../module.h"
#include "../bytereader.h"
#include "../vm.h"

namespace zenith
{
	namespace runtime
	{
		Function::Function(unsigned long loc)
		{
			this->loc = loc;
		}

		void Function::invoke(VMState *state)
		{
			state->module->pushFunctionChain(state->stream->position());
			state->readLevel++;

			state->stream->seek(loc);

			// read instructions till function is completed
			while (state->stream != nullptr &&
				(state->stream->position() < state->stream->max()))
			{
				int32_t ins;
				state->stream->read(&ins);
				state->vm->handleInstruction((Instruction)ins, state->module);
				
				if (ins == CMD_LEAVE_FUNCTION)
					break;
			}
		}

		unsigned long Function::location() const
		{
			return loc;
		}
	}
}
#ifndef __ZENITH_RUNTIME_VM_STATE_H__
#define __ZENITH_RUNTIME_VM_STATE_H__

#include <stack>

namespace zenith
{
	namespace runtime
	{
		class VM;
		class Module;
		class ByteReader;

		struct VMState
		{
		public:
			ByteReader *stream;
			VM *vm;
			Module *module;

			int readLevel = -1;

			VMState()
			{
				stream = nullptr;
				vm = nullptr;
				module = nullptr;
			}

			VMState(ByteReader *stream)
			{
				this->stream = stream;
				vm = nullptr;
				module = nullptr;
			}
		};
	}
}

#endif
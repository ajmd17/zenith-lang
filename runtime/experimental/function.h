#ifndef __ZENITH_RUNTIME_FUNCTION_H__
#define __ZENITH_RUNTIME_FUNCTION_H__

#include <memory>

#include "object.h"
#include "vm_state.h"

namespace zenith
{
	namespace runtime
	{
		class Function : public Object
		{
		private:
			unsigned long loc;

		public:
			Function(unsigned long);

			void invoke(VMState *state);
			unsigned long location() const;
		};

		typedef std::shared_ptr<Function> FunctionPtr;
	}
}

#endif
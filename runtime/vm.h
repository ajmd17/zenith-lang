#ifndef __ZENITH_RUNTIME_VM_H__
#define __ZENITH_RUNTIME_VM_H__

#include <stack>
#include <map>
#include <vector>
#include <memory>
#include <fstream>

#include "module.h"
#include "value.h"
#include "bytereader.h"
#include "exception.h"
#include "../interop/class.h"
#include "../interop/function.h"

namespace zenith
{
	namespace runtime
	{
		typedef std::stack<ValuePtr> ObjectStack;

		class VM
		{
		private:
			Value *lastObject;
			std::vector<ObjectStack> objectStacks;

			std::map<std::string, std::unique_ptr<NativeFunctionBase>> nativeFunctions;
			std::map<std::string, std::unique_ptr<NativeClassBase>> nativeClasses;

			ByteReader *byteReader;

			int blockLevel, readLevel;

			void instruction(Instruction ins, std::unique_ptr<Module> &module);

			ObjectStack &getObjectStack(int id);

		public:
			VM(ByteReader *byteReader);
			~VM();

			void exec();

			template <typename T>
			std::unique_ptr<NativeClass<T>> &bindClass(const std::string &classIdentifier)
			{
				nativeClasses[classIdentifier] = std::make_unique<NativeClass<T>>();
				return nativeClasses[classIdentifier];
			}

			/* Bind a function with no parameters */
			template <typename R>
			void bindFunction(const std::string &identifier, R(*fnPtr)())
			{
				nativeFunctions[identifier] = std::make_unique<NativeFunction_NoParams<R>>(fnPtr);
			}

			/* Bind a function with one parameter */
			template <typename R, typename P1>
			void bindFunction(const std::string &identifier, R(*fnPtr)(P1))
			{
				nativeFunctions[identifier] = std::make_unique<NativeFunction_OneParam<R, P1>>(fnPtr);
			}

			template <typename T>
			void setGlobal(const std::string &identifier, T &&value)
			{
				StackFrame &global = frames[-1];
				ValuePtr vPtr = global.getLocal(identifier);

				if (vPtr == nullptr)
					vPtr = global.createLocal(identifier);

				vPtr->setData(value);
			}

		private:
			bool callBindedFunction(const std::string &identifier, size_t numArgs);
			bool createNativeObject(const std::string &identifier);
		};
	}
}

#endif
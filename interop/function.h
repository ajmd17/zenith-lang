#ifndef __FUNCTION_H__
#define __FUNCTION_H__

#include "../runtime/value.h"

#include <vector>
#include <stack>
#include <iostream>
#include <type_traits>
#include <memory>

namespace zenith
{
	class FunctionUtil
	{
	public:
		// Call a function with no return type and no parameters.
		template <typename R, typename F>
		typename std::enable_if<std::is_void<R>::value, int>::type
			static callFunction(std::stack<ValuePtr> &returnStack, F &functionPtr)
		{
			functionPtr();
			ValuePtr &valuePtr = std::static_pointer_cast<Value>(std::make_shared<Undefined>());
			returnStack.push(valuePtr);

			return 0;
		}

		// Call a function with return type and no parameters.
		template <typename R, typename F>
		typename std::enable_if<!std::is_void<R>::value, int>::type
			static callFunction(std::stack<ValuePtr> &returnStack, F &functionPtr)
		{

			R &&result = functionPtr();
			ValuePtr &valuePtr = std::make_shared<Value>();
			valuePtr->setData(result);
			returnStack.push(valuePtr);

			return 0;
		}

		// Call a function with no return type and 1 parameter.
		template <typename R, typename F, typename P1>
		typename std::enable_if<std::is_void<R>::value, int>::type
			static callFunction(std::stack<ValuePtr> &returnStack, F &functionPtr, P1 &param1)
		{
			functionPtr(param1);
			ValuePtr &valuePtr = std::static_pointer_cast<Value>(std::make_shared<Undefined>());
			returnStack.push(valuePtr);

			return 0;
		}

		// Call a function with return type and 1 parameter.
		template <typename R, typename F, typename P1>
		typename std::enable_if<!std::is_void<R>::value, int>::type
			static callFunction(std::stack<ValuePtr> &returnStack, F &functionPtr, P1 &param1)
		{
			R &&result = functionPtr(param1);
			ValuePtr &valuePtr = std::make_shared<Value>();
			valuePtr->setData(result);
			returnStack.push(valuePtr);

			return 0;
		}
	};

	/* =========================================================== */
	/// Defines a function call, can be stored in containers and then later, casted.
	class NativeFunctionBase
	{
	protected:
		size_t numParams;
	public:
		NativeFunctionBase(size_t numParams = 0) { this->numParams = numParams; }

		size_t getNumParams() const { return numParams; }

		virtual int f(std::stack<ValuePtr> &paramStack, std::stack<ValuePtr> &returnStack) { return 0; }
	};

	/* =========================================================== */
	/// Defines a function with an empty parameter list.
	template <typename R>
	class NativeFunction_NoParams : public NativeFunctionBase
	{
	public:
		NativeFunction_NoParams(R(*fnPtr)()) 
			: NativeFunctionBase(0) { this->fnPtr = fnPtr; }

		int f(std::stack<ValuePtr> &paramStack, std::stack<ValuePtr> &returnStack)
		{
			return FunctionUtil::callFunction<R>(returnStack, fnPtr);
		}
	private:
		R(*fnPtr)();
	};

	/* =========================================================== */
	/// Defines a function with one parameter.
	template <typename R, typename P1>
	class NativeFunction_OneParam : public NativeFunctionBase
	{
	public:
		NativeFunction_OneParam(R(*fnPtr)(P1)) 
			: NativeFunctionBase(1) { this->fnPtr = fnPtr; }

		int f(std::stack<ValuePtr> &paramStack, std::stack<ValuePtr> &returnStack)
		{
			// objPtr cannot be a reference because when objectStack.pop() is called, 
			// the shared_ptr is deleted
			ValuePtr objPtr = paramStack.top();

			if (objPtr->isNative())
			{
				auto base = objPtr->getData<std::shared_ptr<NativeObjectBase>>();
				auto derived = std::dynamic_pointer_cast<NativeObject<std::remove_reference<P1>::type>>(base);
				paramStack.pop();

				return FunctionUtil::callFunction<R>(returnStack, fnPtr, derived->getObject());
			}
			else
			{
				P1 &param1 = objPtr->getData<P1&>();
				paramStack.pop();

				return FunctionUtil::callFunction<R>(returnStack, fnPtr, param1);
			}
		}
	private:
		R(*fnPtr)(P1);
	};

	template <typename O, typename F>
	class NativeMemberFunction_NoParams : public NativeFunctionBase
	{
	public:
		NativeMemberFunction_NoParams(O obj, F fnPtr)
		{
			this->obj = obj;
			this->fnPtr = fnPtr;
		}

		int f(std::stack<ValuePtr> &objectStack)
		{
			(obj.*fnPtr)();
			return 0;
		}
	private:
		O obj;
		F fnPtr;
	};
}

#endif

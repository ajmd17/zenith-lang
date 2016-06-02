#ifndef __ZENITH_RUNTIME_EVALUATOR_H__
#define __ZENITH_RUNTIME_EVALUATOR_H__

#include <string>
#include <memory>
#include <stack>

#include "value.h"

namespace zenith
{
	namespace runtime
	{
		class Object;
		typedef std::shared_ptr<Object> ObjectPtr;

		typedef std::stack<ObjectPtr> ExpressionStack;
		typedef Object &(Object::*BinaryOp)(Object *other);
		typedef Object &(Object::*UnaryOp)();

		class Evaluator
		{
		private:
			ExpressionStack exprStack;

		public:
			ExpressionStack &getStack() { return exprStack; }

			void push(ExpressionStack &whereTo);
			void clear();

			void loadInteger(long value);
			void loadFloat(double value);
			void loadString(const std::string &value);
			void loadObject(ObjectPtr &object);
			void loadNull();

			void assign();
			void assign(BinaryOp op);

			void operation(BinaryOp op);
			void operation(UnaryOp op);
		};
	}
}

#endif
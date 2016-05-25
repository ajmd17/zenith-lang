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
		typedef std::stack<ValuePtr> ExpressionStack;
		typedef Value &(Value::*BinaryOp)(Value *other);
		typedef Value &(Value::*UnaryOp)();

		class Evaluator
		{
		private:
			ExpressionStack exprStack;

		public:
			ExpressionStack &getStack() { return exprStack; }

			void push(std::stack<ValuePtr> &objectStack);
			void clear();

			void loadInteger(long value);
			void loadFloat(double value);
			void loadString(const std::string &value);
			void loadVariable(ValuePtr &value);
			void loadNull();

			void assign();
			void assign(BinaryOp op);

			void operation(BinaryOp op);
			void operation(UnaryOp op);
		};
	}
}

#endif
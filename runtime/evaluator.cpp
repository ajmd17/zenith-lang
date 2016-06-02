#include "evaluator.h"

#include "exception.h"
#include "experimental/object.h"

namespace zenith
{
	namespace runtime
	{
		void Evaluator::push(ExpressionStack &whereTo)
		{
			if (exprStack.size() == 0)
				throw std::runtime_error("Empty stack");

			auto &res = exprStack.top();
			whereTo.push(res);
			clear();
		}

		void Evaluator::clear()
		{
			exprStack = ExpressionStack();
		}

		void Evaluator::loadInteger(long value)
		{
			auto val = std::make_shared<Object>();
			val->assign(value);
			val->setConst(true);
			exprStack.push(val);
		}

		void Evaluator::loadFloat(double value)
		{
			auto val = std::make_shared<Object>();
			val->assign(value);
			val->setConst(true);
			exprStack.push(val);
		}

		void Evaluator::loadString(const std::string &value)
		{
			auto val = std::make_shared<Object>();
			val->assign(value);
			val->setConst(true);
			exprStack.push(val);
		}

		void Evaluator::loadObject(ObjectPtr &object)
		{
			exprStack.push(object);
		}

		void Evaluator::loadNull()
		{
			exprStack.push(ObjectPtr(nullptr));
		}

		void Evaluator::assign()
		{
			auto right = exprStack.top();
			exprStack.pop();

			if (!right)
			{
				auto &left = exprStack.top();
				left->assign(nullptr);
			}
			else
			{
				auto left = exprStack.top();
				exprStack.pop();

				NullValueUsedException().display_if(left == nullptr);
				ConstValueChangedException().display_if(left->isConst());

				Object::assignCopy(left, right);
				exprStack.push(left);
			}
		}

		void Evaluator::assign(BinaryOp op)
		{
			auto right = exprStack.top();
			exprStack.pop();
			auto left = exprStack.top();
			exprStack.pop();

			NullValueUsedException().display_if(left == nullptr);
			ConstValueChangedException().display_if(left->isConst());

			((*left.get()).*op)(right.get());

			exprStack.push(left);
		}

		void Evaluator::operation(BinaryOp op)
		{
			auto right = exprStack.top();
			exprStack.pop();
			auto left = exprStack.top();
			exprStack.pop();

			auto result = std::make_shared<Object>();
			Object::assignCopy(result, left);
			((*result.get()).*op)(right.get());

			exprStack.push(result);
		}

		void Evaluator::operation(UnaryOp op)
		{
			auto top = exprStack.top();
			exprStack.pop();

			auto result = std::make_shared<Object>();
			Object::assignCopy(result, top);
			((*result.get()).*op)();

			exprStack.push(result);
		}
	}
}
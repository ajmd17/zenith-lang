#include "evaluator.h"

namespace zenith
{
	void Evaluator::push(std::stack<ValuePtr> &objectStack)
	{
		if (exprStack.size() == 0)
			throw std::runtime_error("Empty stack");

		ValuePtr &res = exprStack.top();
		objectStack.push(res);
		this->clear();
	}

	void Evaluator::clear()
	{
		exprStack = ExpressionStack();
	}

	void Evaluator::loadInteger(long value)
	{
		auto val = std::make_shared<Value>();
		val->setData(value);
		exprStack.push(val);
	}

	void Evaluator::loadFloat(double value)
	{
		auto val = std::make_shared<Value>();
		val->setData(value);
		exprStack.push(val);
	}

	void Evaluator::loadString(const std::string &value)
	{
		auto val = std::make_shared<Value>();
		val->setData(value);
		exprStack.push(val);
	}

	void Evaluator::loadVariable(ValuePtr &value)
	{
		exprStack.push(value);
	}

	void Evaluator::loadNull()
	{
		exprStack.push(ValuePtr(nullptr));
	}

	void Evaluator::assign()
	{
		ValuePtr right = exprStack.top();
		exprStack.pop();

		// maybe an assign() function for the value
		// could be used here. that way natively binded
		// objects could be assigned, instead of 
		// just being overwritten by the right value.
		if (!right)
		{
			ValuePtr &left = exprStack.top();
			left->setData(nullptr);
		}
		else
		{
			ValuePtr left = exprStack.top();
			exprStack.pop();

			left->set(*right.get());
			exprStack.push(left);
		}

	}

	void Evaluator::assign(BinaryOp op)
	{
		ValuePtr right = exprStack.top();
		exprStack.pop();
		ValuePtr left = exprStack.top();
		exprStack.pop();

		((*left.get()).*op)(right.get());

		exprStack.push(left);
	}

	void Evaluator::operation(BinaryOp op)
	{
		ValuePtr right = exprStack.top();
		exprStack.pop();
		ValuePtr left = exprStack.top();
		exprStack.pop();

		auto result = std::make_shared<Value>();
		result->set(*left.get());
		((*result.get()).*op)(right.get());

		exprStack.push(result);
	}

	void Evaluator::operation(UnaryOp op)
	{
		ValuePtr top = exprStack.top();
		exprStack.pop();

		auto result = std::make_shared<Value>();
		result->set(*top.get());
		((*result.get()).*op)();

		exprStack.push(result);
	}
}
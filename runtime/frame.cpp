#include "frame.h"

#include <algorithm>

#define VALUE_SEARCH_CHECKS 1

namespace zenith
{
	StackFrame::~StackFrame()
	{
		locals.clear();
	}

	Evaluator &StackFrame::getEvaluator()
	{
		return evaluator;
	}

	bool StackFrame::hasLocal(const std::string &identifier)
	{
		auto elt = std::find_if(locals.begin(), locals.end(),
			[&identifier](const std::pair<std::string, ValuePtr> &element)
		{
			return element.first == identifier;
		});

		return elt != locals.end();
	}

	ValuePtr &StackFrame::getLocal(const std::string &identifier)
	{
		auto elt = std::find_if(locals.begin(), locals.end(),
			[&identifier](const std::pair<std::string, ValuePtr> &element)
		{
			return element.first == identifier;
		});

		if (elt->second != nullptr)
			return elt->second;
		
		return ValuePtr(nullptr);
	}

	ValuePtr StackFrame::createLocal(const std::string &identifier)
	{
		#if VALUE_SEARCH_CHECKS
		if (hasLocal(identifier))
			throw std::runtime_error("Value already created");
		#endif

		auto local = std::make_shared<Value>();
		locals.push_back({ identifier, local });
		return local;
	}

	void StackFrame::clearLocal(const std::string &identifier)
	{
		#if VALUE_SEARCH_CHECKS
		if (!hasLocal(identifier))
			throw std::runtime_error("Value does not exist");
		#endif

		auto &val = getLocal(identifier);
		clearLocal(val);
	}

	void StackFrame::clearLocal(ValuePtr &val)
	{
		if (val != nullptr)
		{
			val.reset();
			val = nullptr;
		}
	}

	void StackFrame::deleteLocal(const std::string &identifier)
	{
		auto elt = std::find_if(locals.begin(), locals.end(),
			[&identifier](const std::pair<std::string, ValuePtr> &element)
		{
			return element.first == identifier;
		});

		if (elt != locals.end())
			locals.erase(elt);
		else
			throw std::runtime_error("Value does not exist");
	}
}
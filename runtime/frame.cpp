#include "frame.h"

#include <algorithm>

#include "experimental/object.h"
#include "experimental/function.h"

#define VALUE_SEARCH_CHECKS 1

namespace zenith
{
	namespace runtime
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
				[&identifier](const std::pair<std::string, ObjectPtr> &element)
			{
				return element.first == identifier;
			});

			return elt != locals.end();
		}

		ObjectPtr &StackFrame::getLocal(const std::string &identifier)
		{
			auto elt = std::find_if(locals.begin(), locals.end(),
				[&identifier](const std::pair<std::string, ObjectPtr> &element)
			{
				return element.first == identifier;
			});

			if (elt->second != nullptr)
				return elt->second;

			throw std::runtime_error("Value does not exist");
		}

		ObjectPtr StackFrame::createLocal(const std::string &identifier)
		{
			#if VALUE_SEARCH_CHECKS
			if (hasLocal(identifier))
				throw std::runtime_error("Value already created");
			#endif

			auto local = std::make_shared<Object>();
			locals.push_back({ identifier, local });
			return local;
		}

		ObjectPtr StackFrame::createFunction(const std::string &identifier, unsigned long position)
		{
			#if VALUE_SEARCH_CHECKS
			if (hasLocal(identifier))
				throw std::runtime_error("Value already created");
			#endif

			auto local = std::make_shared<Function>(position);
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

		void StackFrame::clearLocal(ObjectPtr &val)
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
				[&identifier](const std::pair<std::string, ObjectPtr> &element)
			{
				return element.first == identifier;
			});

			if (elt != locals.end())
				locals.erase(elt);
			else
				throw std::runtime_error("Value does not exist");
		}
	}
}
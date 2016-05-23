#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <string>
#include <vector>
#include <map>
#include <stack>
#include <memory>
#include <utility>

#include "value.h"
#include "evaluator.h"

namespace zenith
{
	class StackFrame
	{
	private:
		bool lastIfResult;
		std::vector<std::pair<std::string, ValuePtr>> locals;
		
		Evaluator evaluator;

	public:
		StackFrame() { }
		~StackFrame();

		bool getLastIfResult() const { return lastIfResult; }
		void setLastIfResult(bool b) { lastIfResult = b; }

		Evaluator &getEvaluator();

		bool hasLocal(const std::string &identifier);
		ValuePtr &getLocal(const std::string &identifier);
		ValuePtr createLocal(const std::string &identifier);
		void clearLocal(const std::string &identifier);
		void clearLocal(ValuePtr &val);
		void deleteLocal(const std::string &identifier);
	};
}

#endif
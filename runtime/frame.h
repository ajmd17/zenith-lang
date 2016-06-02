#ifndef __ZENITH_RUNTIME_FRAME_H__
#define __ZENITH_RUNTIME_FRAME_H__

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
	namespace runtime
	{
		class Object;
		typedef std::shared_ptr<Object> ObjectPtr;

		class StackFrame
		{
		private:
			bool lastIfResult;
			std::vector<std::pair<std::string, ObjectPtr>> locals;

			Evaluator evaluator;

		public:
			StackFrame() { }
			~StackFrame();

			bool getLastIfResult() const { return lastIfResult; }
			void setLastIfResult(bool b) { lastIfResult = b; }

			Evaluator &getEvaluator();

			bool hasLocal(const std::string &identifier);
			ObjectPtr &getLocal(const std::string &identifier);
			ObjectPtr createLocal(const std::string &identifier);
			void clearLocal(const std::string &identifier);
			void clearLocal(ObjectPtr &val);
			void deleteLocal(const std::string &identifier);
		};
	}
}

#endif
#include "object.h"

#include <functional>

#include "../exception.h"

namespace zenith
{
	namespace runtime
	{
		Object::~Object()
		{
			methods.clear();
			members.clear();
		}

		void Object::addMethod(const std::string &name, FunctionPtr function)
		{
			if (methods.find(name) != methods.end())
				throw std::runtime_error("Method already exists");

			methods.insert({name, function});
		}

		void Object::invokeMethod(VMState *state, const std::string &name)
		{
			methods[name]->invoke(state);
		}

		void Object::addMember(const std::string &name, ObjectPtr member)
		{
			if (members.find(name) != members.end())
				throw std::runtime_error("Member already exists");

			members.insert({name, member});
		}

		ObjectPtr Object::accessMember(const std::string &name)
		{
			if (members.find(name) == members.end())
				throw std::runtime_error("Member does not exist");

			return members[name];
		}

		ObjectPtr Object::clone()
		{
			auto result = std::make_shared<Object>();
			// copy all members

			for (auto &&member : members)
				result->addMember(member.first, member.second->clone());

			for (auto &&method : methods)
				result->addMethod(method.first, method.second);

			result->any = any;

			return result;
		}

		ObjectPtr Object::assignReference(ObjectPtr left, ObjectPtr right)
		{
			left = right;
			return left;
		}

		ObjectPtr Object::assignCopy(ObjectPtr left, ObjectPtr right)
		{
			left->any = right->any;

			left->_isConst = false; // not const by default
			left->_isNative = right->_isNative;
			left->_isFloat = right->_isFloat;
			left->_isInteger = right->_isInteger;
			left->_isString = right->_isString;
			left->_isObject = right->_isObject;


			return left;
		}

		Object &Object::add(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 + v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 + v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 + v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 + v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString)
			{
				auto &str1 = any.value<std::string&>();
				auto str2 = other->str();

				str1 = str1 + str2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::sub(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 - v2;
			}
			else if (_isInteger && other->isFloat())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<double>();

				v1 = v1 - v2;
			}
			else if (_isFloat && other->isFloat())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<double>();

				v1 = v1 - v2;
			}
			else if (_isFloat && other->isInteger())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<long>();

				v1 = v1 - v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::mul(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 * v2;
			}
			else if (_isInteger && other->isFloat())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<double>();

				v1 = v1 * v2;
			}
			else if (_isFloat && other->isFloat())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<double>();

				v1 = v1 * v2;
			}
			else if (_isFloat && other->isInteger())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<long>();

				v1 = v1 * v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::pow(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = std::pow(v1, v2);
			}
			else if (_isInteger && other->isFloat())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<double>();

				v1 = std::pow(v1, v2);
			}
			else if (_isFloat && other->isFloat())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<double>();

				v1 = std::pow(v1, v2);
			}
			else if (_isFloat && other->isInteger())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<long>();

				v1 = std::pow(v1, v2);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::div(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 / v2;
			}
			else if (_isInteger && other->isFloat())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<double>();

				v1 = v1 / v2;
			}
			else if (_isFloat && other->isFloat())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<double>();

				v1 = v1 / v2;
			}
			else if (_isFloat && other->isInteger())
			{
				auto &v1 = cast<double&>();
				auto v2 = other->cast<long>();

				v1 = v1 / v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::mod(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 % v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::bitxor(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 ^ v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::bitand(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 & v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::bitor(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 | v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::logand(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 && v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::logor(Object *other)
		{
			if (_isInteger && other->isInteger())
			{
				auto &v1 = cast<long&>();
				auto v2 = other->cast<long>();

				v1 = v1 || v2;
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::eql(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 == v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 == v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 == v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 == v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 == str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::not_eql(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 != v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 != v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 != v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 != v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 != str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::less(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 < v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 < v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 < v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 < v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 < str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::greater(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 > v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 > v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 > v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 > v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();

				return *this;
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 > str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::less_eql(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 <= v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 <= v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 <= v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 <= v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 <= str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::greater_eql(Object *other)
		{
			if (isArithmetic())
			{
				if (_isInteger && other->isInteger())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<long>();

					v1 = v1 >= v2;
				}
				else if (_isInteger && other->isFloat())
				{
					auto &v1 = cast<long&>();
					auto v2 = other->cast<double>();

					v1 = v1 >= v2;
				}
				else if (_isFloat && other->isFloat())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<double>();

					v1 = v1 >= v2;
				}
				else if (_isFloat && other->isInteger())
				{
					auto &v1 = cast<double&>();
					auto v2 = other->cast<long>();

					v1 = v1 >= v2;
				}
				else
					BinaryOperatorException({ type_str(), other->type_str() }).display();
			}
			else if (_isString && other->isString())
			{
				auto &str1 = any.value<std::string&>();
				auto &str2 = other->any.value<std::string&>();

				bool result = str1 >= str2;
				any.assign(result);
			}
			else
				BinaryOperatorException({ type_str(), other->type_str() }).display();

			return *this;
		}

		Object &Object::lognot()
		{
			if (_isInteger)
			{
				auto &v1 = cast<long&>();

				v1 = !v1;
			}
			else if (_isFloat)
			{
				auto &v1 = cast<double&>();

				v1 = !v1;
			}
			else
				UnaryOperatorException({ type_str() }).display();

			return *this;
		}

		Object &Object::u_minus()
		{
			if (_isInteger)
			{
				auto &v1 = cast<long&>();

				v1 = -v1;
			}
			else if (_isFloat)
			{
				auto &v1 = cast<double&>();

				v1 = -v1;
			}
			else
				UnaryOperatorException({ type_str() }).display();

			return *this;
		}

		Object &Object::pre_inc()
		{
			throw;
		}

		Object &Object::pre_dec()
		{
			throw;
		}

		Object &Object::post_inc()
		{
			throw;
		}

		Object &Object::post_dec()
		{
			throw;
		}

		std::string Object::str() const
		{
			if (this == nullptr || any.is_null())
				return type_str();
			else if (_isString)
				return any.value<std::string>();
			else if (_isFloat || _isInteger)
			{
				if (any.type() == typeid(double))
					return std::to_string(any.value<double>());
				else if (any.type() == typeid(long))
					return std::to_string(any.value<long>());
				else if (any.type() == typeid(unsigned long))
					return std::to_string(any.value<unsigned long>());
				else if (any.type() == typeid(unsigned int))
					return std::to_string(any.value<unsigned int>());
				else
					return std::to_string(any.value<int>());
			}
			else
				return type_str();
		}

		std::string Object::type_str() const
		{
			std::string result;

			if (this == nullptr)
				return "nullptr";
			else if (any.is_null())
				return "nullval";

			if (_isConst)
				result += "const ";

			if (_isString)
				result += "string";
			else if (_isFloat)
				result += "float";
			else if (_isInteger)
				result += "integer";
			else
				result += any.type().name();

			return result;
		}
	}
}
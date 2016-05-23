#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <memory>
#include <iostream>
#include <type_traits>
#include <map>
using std::cout;

#include <boost/any.hpp>

#include "../enums.h"

namespace zenith
{
	template<class T>
	struct is_c_str
		: std::integral_constant<
		bool,
		std::is_same<char const *, typename std::decay<T>::type>::value ||
		std::is_same<char *, typename std::decay<T>::type>::value
		> {};

	class ValueUtil
	{
	public:
		// for numbers
		template <typename T>
		typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, bool>::type
			static changeType(boost::any &inVal)
		{
			const std::type_info *thisType = &inVal.type();
			const std::type_info *otherType = &typeid(T);

			bool equalTypes = (*otherType) == (*thisType);

			if (equalTypes)
				return true;
			else if ((*thisType) == typeid(double))
			{
				const double v = boost::any_cast<double>(inVal);

				// reassign type
				inVal = static_cast<T>(v);
				return true;
			}
			else if ((*thisType) == typeid(long double))
			{
				const long double v = boost::any_cast<long double>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(float))
			{
				const float v = boost::any_cast<float>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(bool))
			{
				const bool v = boost::any_cast<bool>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(int))
			{
				const int v = boost::any_cast<int>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(unsigned int))
			{
				const unsigned int v = boost::any_cast<unsigned int>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(short))
			{
				const short v = boost::any_cast<short>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(unsigned short))
			{
				const unsigned short v = boost::any_cast<unsigned short>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(char))
			{
				const char v = boost::any_cast<char>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(unsigned char))
			{
				const unsigned char v = boost::any_cast<unsigned char>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(long))
			{
				const long v = boost::any_cast<long>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(unsigned long))
			{
				const unsigned long v = boost::any_cast<unsigned long>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(long long))
			{
				const long long v = boost::any_cast<long long>(inVal);

				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else if ((*thisType) == typeid(unsigned long long))
			{
				const unsigned long long v = boost::any_cast<unsigned long long>(inVal);
				// reassign type
				inVal = static_cast<T>(v);

				return true;
			}
			else
			{
				return false;
			}
		}

		// for objects
		template <typename T>
		typename std::enable_if<!(std::is_integral<T>::value || std::is_floating_point<T>::value), bool>::type
			static changeType(boost::any &inVal)
		{
			const std::type_info *thisType = &inVal.type();
			const std::type_info *otherType = &typeid(T);

			bool equalTypes = (*otherType) == (*thisType);

			if (equalTypes)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	};

	class Value
	{
	protected:
		boost::any value;
		bool isUndefinedValue;
	private:
		bool isNativeObject;
		bool isPointerValue;
		bool isConstValue;
		bool isNumberValue;
		bool isStringValue;
		bool isObjectValue;

	public:
		Value()
		{
			this->value = 0;
			this->isUndefinedValue = true;
			this->isNativeObject = false;
			this->isPointerValue = false;
			this->isConstValue = false;
			this->isNumberValue = false;
			this->isStringValue = false;
			this->isObjectValue = false;
		}

		~Value()
		{
		}

		void set(Value &other)
		{
			if (!this)
			{
				cout << "Error: this was nullptr\n";
				throw std::runtime_error("this == nullptr");
			}

			if (isConstValue)
			{
				cout << "Error: cannot change const value\n";
				throw std::runtime_error("Const changed");
			}

			// if the value is a number or string, we shouldn't assign it as a reference by default.
			// however, it it is an object, it we should use swap()
			bool shouldAssignReference = other.isObject();

			if (shouldAssignReference)
				this->value.swap(other.value);
			else
				this->value = other.value;

			this->isNativeObject = other.isNative();
			this->isUndefinedValue = other.isUndefined();
			this->isPointerValue = other.isPointer();
			this->isConstValue = other.isConst();
			this->isNumberValue = other.isNumber();
			this->isStringValue = other.isString();
			this->isObjectValue = other.isObject();
		}

		template <class T>
		T getData()
		{
			if (this == nullptr || isNull())
			{
				cout << "getData() used on null value\n";
				throw std::runtime_error("Value was null");
			}

			if (this->isPointerValue)
			{
				if (!std::is_pointer<T>::value)
				{
					// attempt to convert the non-pointer version of this object is compatible with the type requested
					auto t = boost::any_cast<std::remove_reference<T>::type*>(value);
					return *t;
				}
			}

			const std::type_info *thisType = &this->value.type();
			const std::type_info *otherType = &typeid(T);

			if (!ValueUtil::changeType<std::remove_reference<T>::type>(this->value))
			{
				cout << "No conversion found between type: (" << thisType->name() << ") and (" << otherType->name() << ")\n";
				throw std::runtime_error("No conversion found");
			}

			auto *t = boost::any_cast<std::remove_reference<T>::type>(&value);
			return *t;
		}

		boost::any &getData()
		{
			return this->value;
		}

		template <class T>
		void setData(T &&value, bool is_native = false)
		{
			static_assert(!is_c_str<T>::value, "C-style strings not supported, please use std::string");

			if (isConstValue)
			{
				cout << "Error: cannot change const value\n";
				throw std::runtime_error("Const changed");
			}

			this->value = value;
			this->isNativeObject = is_native;
			this->isUndefinedValue = false;
			this->isPointerValue = std::is_pointer<T>::value;
			this->isConstValue = std::is_const<T>::value;
			this->isNumberValue = std::is_arithmetic<std::remove_reference<std::remove_pointer<T>::type>::type>::value;
			this->isStringValue = std::is_same<std::remove_const<std::remove_reference<std::remove_pointer<T>::type>::type>::type, std::string>::value;
			this->isObjectValue = !isStringValue && std::is_class<std::remove_reference<std::remove_pointer<T>::type>::type>::value;
		}

		void setUndefined(bool b) { isUndefinedValue = b; }
		void setConst(bool b) { isConstValue = b; }

		const bool isNull() const 
		{ 
			return *getType() == typeid(nullptr); 
		}

		const bool isUndefined() const
		{
			return this->isUndefinedValue;
		}

		const bool isNative() const
		{
			return this->isNativeObject;
		}

		const bool isConst() const
		{
			return this->isConstValue;
		}

		const bool isNumber() const
		{
			return this->isNumberValue;
		}

		const bool isString() const
		{
			return this->isStringValue;
		}

		const bool isObject() const
		{
			return this->isObjectValue;
		}

		const bool isPointer() const
		{
			return this->isPointerValue;
		}

		Value &add(Value *other);
		Value &subtract(Value *other);
		Value &multiply(Value *other);
		Value &power(Value *other);
		Value &divide(Value *other);
		Value &modulus(Value *other);
		Value &bitwiseXor(Value *other);
		Value &bitwiseAnd(Value *other);
		Value &bitwiseOr(Value *other);
		Value &logicalAnd(Value *other);
		Value &logicalOr(Value *other);
		Value &isEqualTo(Value *other);
		Value &notEqualTo(Value *other);
		Value &lessThan(Value *other);
		Value &greaterThan(Value *other);
		Value &lessOrEqualTo(Value *other);
		Value &greaterOrEqualTo(Value *other);
		Value &unaryNot();
		Value &unaryMinus();
		Value &increment();
		Value &decrement();

		std::string str()
		{
			if (this == nullptr || isNull())
				return prettyType();
			else if (isUndefinedValue)
				return prettyType();
			else if (isStringValue)
				return getData<std::string>();
			else if (isNumberValue)
			{
				if (value.type() == typeid(double))
					return std::to_string(getData<double>());
				else if (value.type() == typeid(long))
					return std::to_string(getData<long>());
				else if (value.type() == typeid(unsigned long))
					return std::to_string(getData<unsigned long>());
				else if (value.type() == typeid(unsigned int))
					return std::to_string(getData<unsigned int>());
				else
					return std::to_string(getData<int>());
			}
			else
				return prettyType();
		}

		std::string prettyType() const
		{
			std::string result;

			if (this == nullptr || isNull())
				return "null";


			if (isConstValue)
				result += "const ";

			if (isUndefinedValue)
				result += "undefined";
			else if (isStringValue)
				result += "string";
			else if (isNumberValue)
				result += "number";
			else if (isObjectValue)
				result += "object " + *getType()->name();
			else
				result += *getType()->name();

			return result;
		}

		const std::type_info *getType() const
		{
			return &value.type();
		}
	};

	class Undefined : public Value
	{
	public:
		Undefined()
		{
			this->isUndefinedValue = true;
		}
	};

	struct VarInfo
	{
		std::string varName;
		std::string varTypeStr;

		VarType varType;
		int localBlock = -1;

		VarInfo(int localBlock = -1, VarType varType = VarType::VAR_TYPE_ANY);
	};

	typedef std::shared_ptr<Value> ValuePtr;
	typedef std::reference_wrapper<ValuePtr> ValuePtrRef;
	typedef std::map<std::string, ValuePtr> ValuePtrMap;
	typedef std::pair<std::string, ValuePtr> ValueNamePair;

	typedef std::map<std::string, VarInfo> VarMap;
	typedef std::pair<std::string, VarInfo> VarNameInfoPair;
}

#endif

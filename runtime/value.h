#ifndef __ZENITH_RUNTIME_VALUE_H__
#define __ZENITH_RUNTIME_VALUE_H__

/*#include <string>
#include <memory>
#include <iostream>
#include <type_traits>
#include <map>
using std::cout;

//#include <boost/any.hpp>
#include "any.h"

#include "../enums.h"

namespace zenith
{
	namespace runtime
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
				static changeType(Any &inVal)
			{
				if (inVal.type() == typeid(T))
					return true;

				else if (inVal.compatible<double>())
				{
					const double v = inVal.value<double>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<long double>())
				{
					const long double v = inVal.value<long double>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<float>())
				{
					const float v = inVal.value<float>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<bool>())
				{
					const bool v = inVal.value<bool>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<int>())
				{
					const int v = inVal.value<int>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<unsigned int>())
				{
					const unsigned int v = inVal.value<unsigned int>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<short>())
				{
					const short v = inVal.value<short>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<unsigned short>())
				{
					const unsigned short v = inVal.value<unsigned short>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<char>())
				{
					const char v = inVal.value<char>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<unsigned char>())
				{
					const unsigned char v = inVal.value<unsigned char>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<long>())
				{
					const long v = inVal.value<long>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<unsigned long>())
				{
					const unsigned long v = inVal.value<unsigned long>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<long long>())
				{
					const long long v = inVal.value<long long>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else if (inVal.compatible<unsigned long long>())
				{
					const unsigned long long v = inVal.value<unsigned long long>();
					inVal.assign(static_cast<T>(v));
					return true;
				}
				else
					return false;
			}

			// for objects
			template <typename T>
			typename std::enable_if<!(std::is_integral<T>::value || std::is_floating_point<T>::value), bool>::type
				static changeType(Any &inVal)
			{
				return inVal.type() == typeid(T);
			}
		};

		class Value
		{
		protected:
			Any any;
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
				any.assign(nullptr);
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
				
				this->any = other.any;

				this->isNativeObject = other.isNative();
				this->isUndefinedValue = other.isUndefined();
				this->isPointerValue = other.isPointer();
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

				#if 0
				if (this->isPointerValue)
				{
					if (!std::is_pointer<T>::value)
					{
						// attempt to convert the non-pointer version of this object is compatible with the type requested
						auto *t = value.value<T*>();
						return *t;
					}
				}
				#endif

				const std::type_info *thisType = &this->any.type();
				const std::type_info *otherType = &typeid(T);

				if (!ValueUtil::changeType<std::remove_reference<T>::type>(this->any))
				{
					cout << "No conversion found between type: (" << thisType->name() << ") and (" << otherType->name() << ")\n";
					throw std::runtime_error("No conversion found");
				}

				auto &t = any.value<T&>();
				return t;
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

				this->any.assign(value);
				this->isNativeObject = is_native;
				this->isUndefinedValue = false;
				this->isPointerValue = std::is_pointer<T>::value;
				this->isNumberValue = std::is_arithmetic<
					std::remove_reference<
					std::remove_pointer<T>::type>::type>::value;
				this->isStringValue = std::is_same<
					std::remove_const<
					std::remove_reference<
					std::remove_pointer<T>::type>::type>::type, std::string>::value;
				this->isObjectValue = !isStringValue && std::is_class<
					std::remove_reference<
					std::remove_pointer<T>::type>::type>::value;
			}

			void setUndefined(bool b) { isUndefinedValue = b; }
			void setConst(bool b) { isConstValue = b; }

			const bool isNull() const
			{
				return any.is_null();
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
					if (any.type() == typeid(double))
						return std::to_string(getData<double>());
					else if (any.type() == typeid(long))
						return std::to_string(getData<long>());
					else if (any.type() == typeid(unsigned long))
						return std::to_string(getData<unsigned long>());
					else if (any.type() == typeid(unsigned int))
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
				else
					result += type().name();

				return result;
			}

			const std::type_info &type() const
			{
				return any.type();
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
}*/

#endif

#include "value.h"

#include "exception.h"

namespace zenith
{
	namespace runtime
	{
		Value &Value::add(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double &dbl1 = this->getData<double&>();
				double dbl2 = other->getData<double>();

				dbl1 += dbl2;
			}
			else if (this->isString())
			{
				std::string &str1 = this->getData<std::string&>();
				std::string str2 = other->str();

				str1 += str2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::subtract(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double &dbl1 = this->getData<double&>();
				double dbl2 = other->getData<double>();

				dbl1 -= dbl2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::power(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double &dbl1 = this->getData<double&>();
				double dbl2 = other->getData<double>();

				dbl1 = pow(dbl1, dbl2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::multiply(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double &dbl1 = this->getData<double&>();
				double dbl2 = other->getData<double>();

				dbl1 *= dbl2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::divide(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double &dbl1 = this->getData<double&>();
				double dbl2 = other->getData<double>();

				dbl1 /= dbl2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::modulus(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				int &i1 = this->getData<int&>();
				int i2 = other->getData<int>();

				i1 %= i2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::bitwiseXor(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				int &i1 = this->getData<int&>();
				int i2 = other->getData<int>();

				i1 ^= i2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::bitwiseAnd(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				int &i1 = this->getData<int&>();
				int i2 = other->getData<int>();

				i1 &= i2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::bitwiseOr(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				int &i1 = this->getData<int&>();
				int i2 = other->getData<int>();

				i1 |= i2;
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::logicalAnd(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 && dbl2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::logicalOr(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 || dbl2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::isEqualTo(Value *other)
		{
			Value result;

			if (this != nullptr && !isNull())
			{
				if (other->isNull())
					this->setData(false);

				else if (this->isNumber() && other->isNumber())
				{
					double dbl1 = this->getData<double>();
					double dbl2 = other->getData<double>();

					this->setData(dbl1 == dbl2);
				}
				else if (this->isString() && other->isString())
				{
					std::string str1 = this->getData<std::string>();
					std::string str2 = other->getData<std::string>();

					this->setData(str1 == str2);
				}
				else
					BinaryOperatorException({ prettyType(), other->prettyType() }).display();
			}
			else if (other == nullptr || other->isNull())
				setData(true);
			else if (!(other == nullptr || other->isNull()))
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::notEqualTo(Value *other)
		{
			Value result;

			if (this != nullptr && !isNull())
			{
				if (other == nullptr || other->isNull())
					this->setData(true);

				else if (this->isNumber() && other->isNumber())
				{
					double dbl1 = this->getData<double>();
					double dbl2 = other->getData<double>();

					this->setData(dbl1 != dbl2);
				}
				else if (this->isString() && other->isString())
				{
					std::string str1 = this->getData<std::string>();
					std::string str2 = other->getData<std::string>();

					this->setData(str1 != str2);
				}
				else
					BinaryOperatorException({ prettyType(), other->prettyType() }).display();
			}
			else if (other == nullptr || other->isNull())
				setData(false);
			else if (!(other == nullptr || other->isNull()))
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::lessThan(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 < dbl2);
			}
			else if (this->isString() && other->isString())
			{
				std::string str1 = this->getData<std::string>();
				std::string str2 = other->getData<std::string>();

				this->setData(str1 < str2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::greaterThan(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 > dbl2);
			}
			else if (this->isString() && other->isString())
			{
				std::string str1 = this->getData<std::string>();
				std::string str2 = other->getData<std::string>();

				this->setData(str1 > str2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::lessOrEqualTo(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 <= dbl2);
			}
			else if (this->isString() && other->isString())
			{
				std::string str1 = this->getData<std::string>();
				std::string str2 = other->getData<std::string>();

				this->setData(str1 <= str2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::greaterOrEqualTo(Value *other)
		{
			Value result;

			if (this->isNumber() && other->isNumber())
			{
				double dbl1 = this->getData<double>();
				double dbl2 = other->getData<double>();

				this->setData(dbl1 >= dbl2);
			}
			else if (this->isString() && other->isString())
			{
				std::string str1 = this->getData<std::string>();
				std::string str2 = other->getData<std::string>();

				this->setData(str1 >= str2);
			}
			else
				BinaryOperatorException({ prettyType(), other->prettyType() }).display();

			return *this;
		}

		Value &Value::unaryNot()
		{
			Value result;

			if (isNull() || isUndefined())
			{
				this->setData(true); // !x (where x is null/undefined)
			}
			else if (this->isNumber())
			{
				double dbl1 = this->getData<double>();

				this->setData(!dbl1);
			}
			else if (this->isString())
			{
				Exception({ "Operator not valid on this type: (" + prettyType() + ")" }).display();
			}
			else
			{
				this->setData(false); // non null
			}

			return *this;
		}

		Value &Value::unaryMinus()
		{
			Value result;

			if (this->isNumber())
			{
				double &dbl1 = this->getData<double&>();

				dbl1 *= -1;
			}
			else
			{
				Exception({ "Operator not valid on this type: (" + prettyType() + ")" }).display();
			}

			return *this;
		}

		Value &Value::increment()
		{
			Value result;

			if (this->isNumber())
			{
				double &dbl1 = this->getData<double&>();

				++dbl1;
			}
			else
			{
				Exception({ "Operator not valid on this type: (" + prettyType() + ")" }).display();
			}

			return *this;
		}

		Value &Value::decrement()
		{
			Value result;

			if (this->isNumber())
			{
				double &dbl1 = this->getData<double&>();

				--dbl1;
			}
			else
			{
				Exception({ "Operator not valid on this type: (" + prettyType() + ")" }).display();
			}

			return *this;
		}

		VarInfo::VarInfo(int localBlock, VarType varType)
		{
			this->localBlock = localBlock;
			this->varType = varType;
		}
	}
}
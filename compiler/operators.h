#ifndef __ZENITH_COMPILER_OPERATORS_H__
#define __ZENITH_COMPILER_OPERATORS_H__

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

namespace zenith
{
	namespace compiler
	{
		enum Operator
		{
			OP_INVALID = -1,
			OP_POWER,
			OP_MULTIPLY,
			OP_DIVIDE,
			OP_INT_DIVIDE,
			OP_MODULUS,
			OP_ADD,
			OP_SUBTRACT,
			OP_BIT_XOR,
			OP_BIT_AND,
			OP_BIT_OR,
			OP_NOT,
			OP_AND,
			OP_OR,
			OP_EQUALS,
			OP_NOT_EQUAL,
			OP_OUTPUT,
			OP_INPUT,
			OP_LESS,
			OP_GREATER,
			OP_LESS_OR_EQUAL,
			OP_GREATER_OR_EQUAL,
			OP_INCREMENT,
			OP_DECREMENT,
			OP_ASSIGN,
			OP_ADD_ASSIGN,
			OP_SUBTRACT_ASSIGN,
			OP_MULTIPLY_ASSIGN,
			OP_DIVIDE_ASSIGN,
			OP_MODULUS_ASSIGN,
			OP_BIT_AND_ASSIGN,
			OP_BIT_XOR_ASSIGN,
			OP_BIT_OR_ASSIGN
		};

		static std::map<std::string, Operator> operators =
		{
			{"**", OP_POWER },
			{ "*", OP_MULTIPLY },
			{ "/", OP_INT_DIVIDE },
			{ "\\", OP_DIVIDE },
			{ "%", OP_MODULUS },
			{ "+", OP_ADD },
			{ "-", OP_SUBTRACT },
			{ "^", OP_BIT_XOR },
			{ "&", OP_BIT_AND },
			{ "|", OP_BIT_OR },
			{ "!", OP_NOT },
			{ "&&", OP_AND },
			{ "||", OP_OR },
			{ "==", OP_EQUALS },
			{ "!=", OP_NOT_EQUAL },
			{ "<<", OP_OUTPUT },
			{ ">>", OP_INPUT },
			{ "<", OP_LESS },
			{ ">", OP_GREATER },
			{ "<=", OP_LESS_OR_EQUAL },
			{ ">=", OP_GREATER_OR_EQUAL },
			{ "++", OP_INCREMENT },
			{ "--", OP_DECREMENT },
			{ "=", OP_ASSIGN },
			{ "+=", OP_ADD_ASSIGN },
			{ "-=", OP_SUBTRACT_ASSIGN },
			{ "*=", OP_MULTIPLY_ASSIGN },
			{ "/=", OP_DIVIDE_ASSIGN },
			{ "%=", OP_MODULUS_ASSIGN },
			{ "&=", OP_BIT_AND_ASSIGN },
			{ "^=", OP_BIT_XOR_ASSIGN },
			{ "|=", OP_BIT_OR_ASSIGN }
		};

		static std::map<Operator, int> precedenceMap =
		{
			{ OP_INCREMENT, 14 },{ OP_DECREMENT, 14 },{ OP_NOT, 14 },

			{ OP_POWER, 13 },

			{ OP_MULTIPLY, 12 },{ OP_DIVIDE, 12 },{ OP_MODULUS, 12 },

			{ OP_ADD, 11 },{ OP_SUBTRACT, 11 },

			{ OP_OUTPUT, 10 },{ OP_INPUT, 10 },

			{ OP_GREATER, 9 },{ OP_GREATER_OR_EQUAL, 9 },{ OP_LESS, 9 },{ OP_LESS_OR_EQUAL, 9 },

			{ OP_EQUALS, 8 },{ OP_NOT_EQUAL, 8 },

			{ OP_BIT_AND, 7 },

			{ OP_BIT_XOR, 6 },

			{ OP_BIT_OR, 5 },

			{ OP_AND, 4 },

			{ OP_OR, 3 },

			{ OP_ASSIGN, 2 },
			{ OP_ADD_ASSIGN, 2 },{ OP_SUBTRACT_ASSIGN, 2 },
			{ OP_MULTIPLY_ASSIGN, 2 },{ OP_DIVIDE_ASSIGN, 2 },{ OP_MODULUS_ASSIGN, 2 },
			{ OP_BIT_AND_ASSIGN, 2 },{ OP_BIT_XOR_ASSIGN, 2 },{ OP_BIT_OR_ASSIGN, 2 }
		};

		inline const std::string &getOperatorStr(Operator op)
		{
			auto it = std::find_if(operators.begin(), operators.end(),
				[&op](const std::pair<std::string, Operator> &item) {
				return item.second == op;
			});

			if (it == operators.end())
				throw std::out_of_range("Operator has no string equivalent");

			return it->first;
		}

		inline const Operator getOperator(const std::string &str)
		{
			auto it = operators.find(str);

			if (it == operators.end())
				return Operator::OP_INVALID;
			else
				return it->second;
		}

		inline int getPrecedence(Operator op)
		{
			return precedenceMap.at(op);
		}
	}
}
#endif
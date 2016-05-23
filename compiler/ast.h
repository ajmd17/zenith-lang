#ifndef __AST_H__
#define __AST_H__

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "operators.h"
#include "src_location.h"

namespace zenith
{
	enum AstNodeType
	{
		AST_MODULE,
		AST_IMPORT,
		AST_STATEMENT,
		AST_BLOCK,
		AST_EXPRESSION,
		AST_BINARY_OPERATION,
		AST_UNARY_OPERATION,
		AST_MEMBER_ACCESS,
		AST_VARIABLE_DECLARATION,
		AST_VARIABLE,
		AST_INTEGER,
		AST_FLOAT,
		AST_STRING,
		AST_TRUE,
		AST_FALSE,
		AST_NULL,
		AST_FUNCTION_DEFINITION,
		AST_FUNCTION_CALL,
		AST_IF_STATEMENT,
		AST_RETURN_STATEMENT,
		AST_FOR_LOOP
	};

	struct AstNode;
	struct ModuleAst;
	struct ImportAst;
	struct StatementAst;
	struct BlockAst;
	struct ExpressionAst;
	struct BinaryOperationAst;
	struct UnaryOperationAst;
	struct MemberAccessAst;
	struct VariableDeclarationAst;
	struct VariableAst;
	struct IntegerAst;
	struct FloatAst;
	struct StringAst;
	struct TrueAst;
	struct FalseAst;
	struct NullAst;
	struct FunctionDefinitionAst;
	struct FunctionCallAst;
	struct IfStatementAst;
	struct ReturnStatementAst;
	struct ForLoopAst;

	class AstHandler
	{
	public:
		virtual void accept(ModuleAst *node) = 0;

	protected:
		virtual void accept(AstNode *node) = 0;
		virtual void accept(ImportAst *node) = 0;
		virtual void accept(StatementAst *node) = 0;
		virtual void accept(BlockAst *node) = 0;
		virtual void accept(ExpressionAst *node) = 0;
		virtual void accept(BinaryOperationAst *node) = 0;
		virtual void accept(UnaryOperationAst *node) = 0;
		virtual void accept(MemberAccessAst *node) = 0;
		virtual void accept(VariableDeclarationAst *node) = 0;
		virtual void accept(VariableAst *node) = 0;
		virtual void accept(IntegerAst *node) = 0;
		virtual void accept(FloatAst *node) = 0;
		virtual void accept(StringAst *node) = 0;
		virtual void accept(TrueAst *node) = 0;
		virtual void accept(FalseAst *node) = 0;
		virtual void accept(NullAst *node) = 0;
		virtual void accept(FunctionDefinitionAst *node) = 0;
		virtual void accept(FunctionCallAst *node) = 0;
		virtual void accept(IfStatementAst *node) = 0;
		virtual void accept(ReturnStatementAst *node) = 0;
		virtual void accept(ForLoopAst *node) = 0;
	};

	struct AstNode
	{
	protected:
		static int indentLevel;

		std::string tabIndent(int level, const std::string &str)
		{
			std::string result;
			for (int i = 0; i < level; i++)
				result += "  ";
			result += str;
			return result;
		}
	public:
		SourceLocation location;
		AstNodeType nodeType;

		AstNode(SourceLocation location, AstNodeType nodeType)
		{
			this->location = location;
			this->nodeType = nodeType;
		}

		virtual std::string str() = NULL;
	};

	struct ModuleAst : public AstNode
	{
		std::string moduleName;
		std::vector<std::unique_ptr<AstNode>> children;

		ModuleAst(SourceLocation location, const std::string &moduleName)
			: AstNode(location, AST_MODULE)
		{
			this->moduleName = moduleName;
		}

		void addChild(std::unique_ptr<AstNode> node)
		{
			children.push_back(std::move(node));
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Module (" + moduleName + ")");

			indentLevel++;
			for (auto &&child : children)
			{
				result += "\n";
				result += child != nullptr ?
					child->str() :
					tabIndent(indentLevel, "(null)");
			}
			indentLevel--;

			return result;
		}
	};

	struct StatementAst : public AstNode
	{
		std::vector<std::unique_ptr<AstNode>> children;

		StatementAst(SourceLocation location)
			: AstNode(location, AST_STATEMENT)
		{
		}

		void addChild(std::unique_ptr<AstNode> node)
		{
			children.push_back(std::move(node));
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Statement");

			indentLevel++;
			for (auto &&child : children)
			{
				result += "\n";
				result += child != nullptr ?
					child->str() :
					tabIndent(indentLevel, "(null)");
			}
			indentLevel--;

			return result;
		}
	};

	struct ImportAst : public AstNode
	{
		std::string value;
		std::string localPath;
		bool isModuleImport;

		ImportAst(SourceLocation location,
			const std::string &value,
			const std::string &localPath,
			bool isModuleImport)
			: AstNode(location, AST_IMPORT)
		{
			this->value = value;
			this->localPath = localPath;
			this->isModuleImport = isModuleImport;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, std::string("Import ") + (isModuleImport ? "module" : "file"));

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, value);
			result += "\n";
			result += tabIndent(indentLevel, localPath);
			indentLevel--;

			return result;
		}
	};

	struct BlockAst : public AstNode
	{
		std::vector<std::unique_ptr<AstNode>> children;

		BlockAst(SourceLocation location)
			: AstNode(location, AST_BLOCK)
		{
		}

		void addChild(std::unique_ptr<AstNode> node)
		{
			children.push_back(std::move(node));
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Block");

			indentLevel++;
			for (auto &&child : children)
			{
				result += "\n";
				result += child != nullptr ?
					child->str() :
					tabIndent(indentLevel, "(null)");
			}
			indentLevel--;

			return result;
		}
	};

	struct ExpressionAst : public AstNode
	{
		std::unique_ptr<AstNode> value;
		bool shouldClearStack;

		ExpressionAst(SourceLocation location,
			std::unique_ptr<AstNode> value,
			bool shouldClearStack)
			: AstNode(location, AstNodeType::AST_EXPRESSION)
		{
			this->value = std::move(value);
			this->shouldClearStack = shouldClearStack;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Expression");

			indentLevel++;
			result += "\n";
			if (value != nullptr)
				result += value->str();
			else
				result += tabIndent(indentLevel, "(null)");

			indentLevel--;

			return result;
		}
	};

	struct BinaryOperationAst : public AstNode
	{
		std::unique_ptr<AstNode> left;
		std::unique_ptr<AstNode> right;
		Operator op;

		BinaryOperationAst(SourceLocation location,
			std::unique_ptr<AstNode> left,
			std::unique_ptr<AstNode> right,
			Operator op)
			: AstNode(location, AST_BINARY_OPERATION)
		{
			this->left = std::move(left);
			this->right = std::move(right);
			this->op = op;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "BinaryOperation");

			indentLevel++;
			result += "\n";
			result += left->str();
			result += "\n";
			result += tabIndent(indentLevel, getOperatorStr(op));
			result += "\n";
			result += right->str();
			indentLevel--;

			return result;
		}
	};

	struct UnaryOperationAst : public AstNode
	{
		std::unique_ptr<AstNode> value;
		Operator op;

		UnaryOperationAst(SourceLocation location,
			std::unique_ptr<AstNode> value,
			Operator op)
			: AstNode(location, AST_UNARY_OPERATION)
		{
			this->value = std::move(value);
			this->op = op;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "UnaryOperation");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, getOperatorStr(op));
			result += "\n";
			result += value->str();
			indentLevel--;

			return result;
		}
	};

	struct MemberAccessAst : public AstNode
	{
		std::string identifier;
		std::unique_ptr<AstNode> next;

		MemberAccessAst(SourceLocation location,
			const std::string &identifier,
			std::unique_ptr<AstNode> next)
			: AstNode(location, AST_MEMBER_ACCESS)
		{
			this->next = std::move(next);
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "MemberAccess");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, identifier);
			result += "\n";
			if (next != nullptr)
				result += next->str();
			else
				result += tabIndent(indentLevel, "(null)");
			indentLevel--;

			return result;
		}
	};

	struct VariableDeclarationAst : public AstNode
	{
		std::string name;

		VariableDeclarationAst(SourceLocation location, const std::string &name)
			: AstNode(location, AST_VARIABLE_DECLARATION)
		{
			this->name = name;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "VariableDeclaration");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, name);
			indentLevel--;

			return result;
		}
	};

	struct VariableAst : public AstNode
	{
		std::string name;

		VariableAst(SourceLocation location, const std::string &name)
			: AstNode(location, AST_VARIABLE)
		{
			this->name = name;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Variable");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, name);
			indentLevel--;

			return result;
		}
	};

	struct IntegerAst : public AstNode
	{
		long value;

		IntegerAst(SourceLocation location, long value)
			: AstNode(location, AST_INTEGER)
		{
			this->value = value;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Integer");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, std::to_string(value));
			indentLevel--;

			return result;
		}
	};

	struct FloatAst : public AstNode
	{
		double value;

		FloatAst(SourceLocation location, double value)
			: AstNode(location, AST_FLOAT)
		{
			this->value = value;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Float");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, std::to_string(value));
			indentLevel--;

			return result;
		}
	};

	struct StringAst : public AstNode
	{
		std::string value;

		StringAst(SourceLocation location, const std::string &value)
			: AstNode(location, AST_STRING)
		{
			this->value = value;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "String");

			indentLevel++;
			result += "\n";
			result += tabIndent(indentLevel, value);
			indentLevel--;

			return result;
		}
	};

	struct TrueAst : public AstNode
	{
		TrueAst(SourceLocation location)
			: AstNode(location, AST_TRUE)
		{
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "True");

			return result;
		}
	};

	struct FalseAst : public AstNode
	{
		FalseAst(SourceLocation location)
			: AstNode(location, AST_FALSE)
		{
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "False");

			return result;
		}
	};

	struct NullAst : public AstNode
	{
		NullAst(SourceLocation location)
			: AstNode(location, AST_NULL)
		{
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "Null");

			return result;
		}
	};

	struct FunctionDefinitionAst : public AstNode
	{
		std::string name;
		std::vector<std::string> arguments;
		std::unique_ptr<AstNode> block;
		bool isNative;

		FunctionDefinitionAst(SourceLocation location,
			const std::string &name,
			std::vector<std::string> arguments,
			std::unique_ptr<AstNode> block,
			bool isNative = false)
			: AstNode(location, AST_FUNCTION_DEFINITION)
		{
			this->name = name;
			this->arguments = arguments;
			this->block = std::move(block);
			this->isNative = isNative;
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "FunctionDefinition (" + name + ")");

			indentLevel++;
			for (auto &&arg : arguments)
			{
				result += "\n";
				result += tabIndent(indentLevel, arg);
			}
			if (!isNative)
			{
				result += "\n";
				result += block->str();
			}

			indentLevel--;

			return result;
		}
	};

	struct FunctionCallAst : public AstNode
	{
		std::string name;
		std::vector<std::unique_ptr<AstNode>> arguments;

		FunctionCallAst(SourceLocation location,
			const std::string &name,
			std::vector<std::unique_ptr<AstNode>> arguments)
			: AstNode(location, AST_FUNCTION_CALL)
		{
			this->name = name;
			this->arguments = std::move(arguments);
		}

		void addArgument(std::unique_ptr<AstNode> arg)
		{
			arguments.push_back(std::move(arg));
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "FunctionCall (" + name + ")");

			indentLevel++;
			for (auto &&arg : arguments)
			{
				result += "\n";
				result += arg->str();
			}
			indentLevel--;

			return result;
		}
	};

	struct IfStatementAst : public AstNode
	{
		std::unique_ptr<AstNode> cond_expr, block, elseStatement;

		IfStatementAst(SourceLocation location,
			std::unique_ptr<AstNode> cond_expr,
			std::unique_ptr<AstNode> block,
			std::unique_ptr<AstNode> elseStatement)
			: AstNode(location, AST_IF_STATEMENT)
		{
			this->cond_expr = std::move(cond_expr);
			this->block = std::move(block);
			this->elseStatement = std::move(elseStatement);
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "IfStatement");

			indentLevel++;

			result += "\n";
			result += cond_expr->str();
			result += "\n";
			result += block->str();

			if (elseStatement != nullptr)
			{
				result += "\n";
				result += elseStatement->str();
			}

			indentLevel--;

			return result;
		}
	};

	struct ReturnStatementAst : public AstNode
	{
		std::unique_ptr<AstNode> value;

		ReturnStatementAst(SourceLocation location, std::unique_ptr<AstNode> value)
			: AstNode(location, AstNodeType::AST_RETURN_STATEMENT)
		{
			this->value = std::move(value);
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "ReturnStatement");

			indentLevel++;

			if (value)
			{
				result += "\n";
				result += value->str();
			}

			indentLevel--;

			return result;
		}
	};

	struct ForLoopAst : public AstNode
	{
		std::unique_ptr<AstNode> init_expr;
		std::unique_ptr<AstNode> cond_expr;
		std::unique_ptr<AstNode> inc_expr;

		std::vector<std::unique_ptr<AstNode>> statements;

		ForLoopAst(SourceLocation location,
			std::unique_ptr<AstNode> init_expr,
			std::unique_ptr<AstNode> cond_expr,
			std::unique_ptr<AstNode> inc_expr)
			: AstNode(location, AST_FOR_LOOP)
		{
			this->init_expr = std::move(init_expr);
			this->cond_expr = std::move(cond_expr);
			this->inc_expr = std::move(inc_expr);
		}

		void addStatement(std::unique_ptr<AstNode> statement)
		{
			statements.push_back(std::move(statement));
		}

		std::string str()
		{
			std::string result;
			result += tabIndent(indentLevel, "ForLoop");

			indentLevel++;

			result += "\n";
			result += init_expr->str();
			result += "\n";
			result += cond_expr->str();
			result += "\n";
			result += inc_expr->str();

			indentLevel--;

			return result;
		}
	};
}

#endif
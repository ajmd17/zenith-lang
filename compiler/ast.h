#ifndef __ZENITH_COMPILER_AST_H__
#define __ZENITH_COMPILER_AST_H__

#include <vector>
#include <map>
#include <string>
#include <memory>

#include "operators.h"
#include "src_location.h"

namespace zenith
{
	namespace compiler
	{
		enum AstNodeType
		{
			AST_MODULE,
			AST_IMPORTS,
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
			AST_SELF,
			AST_NEW,
			AST_FUNCTION_DEFINITION,
			AST_FUNCTION_CALL,
			AST_CLASS,
			AST_IF_STATEMENT,
			AST_RETURN_STATEMENT,
			AST_FOR_LOOP
		};

		struct AstNode;
		struct ModuleAst;
		struct ImportsAst;
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
		struct SelfAst;
		struct NewAst;
		struct FunctionDefinitionAst;
		struct FunctionCallAst;
		struct ClassAst;
		struct IfStatementAst;
		struct ReturnStatementAst;
		struct ForLoopAst;

		class AstHandler
		{
		public:
			virtual void accept(ModuleAst *node) = 0;

		protected:
			virtual void accept(AstNode *node) = 0;
			virtual void accept(ImportsAst *node) = 0;
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
			virtual void accept(SelfAst *node) = 0;
			virtual void accept(NewAst *node) = 0;
			virtual void accept(FunctionDefinitionAst *node) = 0;
			virtual void accept(FunctionCallAst *node) = 0;
			virtual void accept(ClassAst *node) = 0;
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
			AstNode *module = nullptr;
			std::pair<std::string, ClassAst*> self = {"", nullptr};

			SourceLocation location;
			AstNodeType nodeType;

			AstNode(SourceLocation location, AstNode *module, AstNodeType nodeType)
			{
				this->location = location;
				this->module = module;
				this->nodeType = nodeType;
			}

			virtual std::string str() = 0;
		};

		struct ModuleAst : public AstNode
		{
			std::string moduleName;
			std::vector<std::unique_ptr<AstNode>> children;

			ModuleAst(SourceLocation location, const std::string &moduleName)
				: AstNode(location, this, AST_MODULE)
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

			StatementAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_STATEMENT)
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

		struct ImportsAst : public AstNode
		{
			std::vector<std::unique_ptr<AstNode>> imports;

			ImportsAst(SourceLocation location,
				AstNode *module,
				std::vector<std::unique_ptr<AstNode>> imports)
				: AstNode(location, module, AST_IMPORTS)
			{
				this->imports = std::move(imports);
			}

			std::string str()
			{
				std::string result;
				result += tabIndent(indentLevel, "Imports");

				indentLevel++;
				for (auto &&child : imports)
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
				AstNode *module,
				const std::string &value,
				const std::string &localPath,
				bool isModuleImport)
				: AstNode(location, module, AST_IMPORT)
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

			BlockAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_BLOCK)
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
				AstNode *module,
				std::unique_ptr<AstNode> value,
				bool shouldClearStack)
				: AstNode(location, module, AstNodeType::AST_EXPRESSION)
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
				AstNode *module,
				std::unique_ptr<AstNode> left,
				std::unique_ptr<AstNode> right,
				Operator op)
				: AstNode(location, module, AST_BINARY_OPERATION)
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
				AstNode *module,
				std::unique_ptr<AstNode> value,
				Operator op)
				: AstNode(location, module, AST_UNARY_OPERATION)
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
			std::string leftStr;
			std::unique_ptr<AstNode> left;
			std::unique_ptr<AstNode> right;

			MemberAccessAst(SourceLocation location, 
				AstNode *module,
				const std::string &leftStr,
				std::unique_ptr<AstNode> left,
				std::unique_ptr<AstNode> right)
				: AstNode(location, module, AST_MEMBER_ACCESS)
			{
				this->leftStr = leftStr;
				this->left = std::move(left);
				this->right = std::move(right);
			}

			std::string str()
			{
				std::string result;
				result += tabIndent(indentLevel, "MemberAccess");

				indentLevel++;
				result += "\n";
				if (left != nullptr)
					result += left->str();
				else
					result += tabIndent(indentLevel, "(null)");

				result += "\n";
				if (right != nullptr)
					result += right->str();
				else
					result += tabIndent(indentLevel, "(null)");

				return result;
			}
		};

		struct VariableDeclarationAst : public AstNode
		{
			std::string name;
			std::unique_ptr<AstNode> assignment;

			VariableDeclarationAst(SourceLocation location,
				AstNode *module,
				const std::string &name,
				std::unique_ptr<AstNode> assignment)
				: AstNode(location, module, AST_VARIABLE_DECLARATION)
			{
				this->name = name;
				this->assignment = std::move(assignment);
			}

			std::string str()
			{
				std::string result;
				result += tabIndent(indentLevel, "VariableDeclaration");

				indentLevel++;
				result += "\n";
				result += tabIndent(indentLevel, name);
				result += "\n";
				result += assignment != nullptr ? 
					assignment->str() : 
					tabIndent(indentLevel, "(null)");
				indentLevel--;

				return result;
			}
		};

		struct VariableAst : public AstNode
		{
			std::string name;

			VariableAst(SourceLocation location, 
				AstNode *module, 
				const std::string &name)
				: AstNode(location, module, AST_VARIABLE)
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

			IntegerAst(SourceLocation location, 
				AstNode *module, 
				long value)
				: AstNode(location, module, AST_INTEGER)
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

			FloatAst(SourceLocation location,
				AstNode *module, 
				double value)
				: AstNode(location, module, AST_FLOAT)
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

			StringAst(SourceLocation location, 
				AstNode *module, 
				const std::string &value)
				: AstNode(location, module, AST_STRING)
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
			TrueAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_TRUE)
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
			FalseAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_FALSE)
			{
			}

			std::string str()
			{
				std::string result;
				indentLevel++;
				result += "\n";
				result += tabIndent(indentLevel, "False");
				indentLevel--;

				return result;
			}
		};

		struct NullAst : public AstNode
		{
			NullAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_NULL)
			{
			}

			std::string str()
			{
				std::string result;
				indentLevel++;
				result += "\n";
				result += tabIndent(indentLevel, "Null");
				indentLevel--;

				return result;
			}
		};

		struct SelfAst : public AstNode
		{
			SelfAst(SourceLocation location, AstNode *module)
				: AstNode(location, module, AST_SELF)
			{
			}

			std::string str()
			{
				std::string result;
				result += "\n";
				result += tabIndent(indentLevel, "Self");

				return result;
			}
		};

		struct NewAst : public AstNode
		{
			std::string identifier;
			std::unique_ptr<AstNode> constructorAst;

			NewAst(SourceLocation location,
				AstNode *module,
				const std::string &identifier, // the variable it will go into
				std::unique_ptr<AstNode> constructorAst)
				: AstNode(location, module, AST_NEW)
			{
				this->identifier = identifier;
				this->constructorAst = std::move(constructorAst);
			}

			std::string str()
			{
				std::string result;
				result += tabIndent(indentLevel, "New -> " + identifier);

				indentLevel++;
				result += "\n";
				result += constructorAst->str();
				indentLevel--;

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
				AstNode *module,
				const std::string &name,
				std::vector<std::string> arguments,
				std::unique_ptr<AstNode> block,
				bool isNative = false)
				: AstNode(location, module, AST_FUNCTION_DEFINITION)
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
				AstNode *module,
				const std::string &name,
				std::vector<std::unique_ptr<AstNode>> arguments)
				: AstNode(location, module, AST_FUNCTION_CALL)
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

		struct ClassAst : public AstNode
		{
			std::string name;
			std::vector<std::unique_ptr<AstNode>> dataMembers;

			ClassAst(SourceLocation location,
				AstNode *module,
				const std::string &name,
				std::vector<std::unique_ptr<AstNode>> dataMembers)
				: AstNode(location, module, AST_CLASS)
			{
				this->name = name;
				this->dataMembers = std::move(dataMembers);
			}

			std::string str()
			{
				std::string result;
				result += tabIndent(indentLevel, "Class (" + name + ")");

				indentLevel++;
				for (auto &&mem : dataMembers)
				{
					result += "\n";
					result += mem->str();
				}
				indentLevel--;

				return result;
			}
		};

		struct IfStatementAst : public AstNode
		{
			std::unique_ptr<AstNode> cond_expr, block, elseStatement;

			IfStatementAst(SourceLocation location, 
				AstNode *module,
				std::unique_ptr<AstNode> cond_expr,
				std::unique_ptr<AstNode> block,
				std::unique_ptr<AstNode> elseStatement)
				: AstNode(location, module, AST_IF_STATEMENT)
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

			ReturnStatementAst(SourceLocation location, 
				AstNode *module, 
				std::unique_ptr<AstNode> value)
				: AstNode(location, module, AstNodeType::AST_RETURN_STATEMENT)
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
			std::unique_ptr<AstNode> block;

			ForLoopAst(SourceLocation location, 
				AstNode *module,
				std::unique_ptr<AstNode> init_expr,
				std::unique_ptr<AstNode> cond_expr,
				std::unique_ptr<AstNode> inc_expr,
				std::unique_ptr<AstNode> block)
				: AstNode(location, module, AST_FOR_LOOP)
			{
				this->init_expr = std::move(init_expr);
				this->cond_expr = std::move(cond_expr);
				this->inc_expr = std::move(inc_expr);
				this->block = std::move(block);
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
				result += "\n";
				result += block->str();

				indentLevel--;

				return result;
			}
		};
	}
}

#endif
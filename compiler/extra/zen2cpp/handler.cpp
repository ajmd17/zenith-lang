#include "handler.h"

#include <sstream>
#include <fstream>
#include <algorithm>

#include "../../lexer.h"
#include "../../parser.h"

// This tool is experimental and should not be used

namespace zenith
{
	namespace compiler
	{
		namespace zen2cpp
		{
			Zen2CppHandler::Zen2CppHandler(ParserState &state)
			{
				this->state = state;
			}

			Zen2CppHandler::~Zen2CppHandler()
			{

			}

			void Zen2CppHandler::accept(ModuleAst *node)
			{
				append("int main() {\n\n");
				levels[++level] = Level();

				for (int i = 0; i < node->children.size(); i++)
					accept(node->children[i].get());

				append("\n\n");
				indent();
				append("return 0;\n");
				--level;
				append("}\n");
			}

			void Zen2CppHandler::accept(AstNode *node)
			{
				if (node == nullptr)
				{
					return;
				}

				switch (node->nodeType)
				{
				case AstNodeType::AST_IMPORTS:
					accept(dynamic_cast<ImportsAst*>(node));
					break;
				case AstNodeType::AST_IMPORT:
					accept(dynamic_cast<ImportAst*>(node));
					break;
				case AstNodeType::AST_STATEMENT:
					accept(dynamic_cast<StatementAst*>(node));
					break;
				case AstNodeType::AST_BLOCK:
					accept(dynamic_cast<BlockAst*>(node));
					break;
				case AstNodeType::AST_EXPRESSION:
					accept(dynamic_cast<ExpressionAst*>(node));
					break;
				case AstNodeType::AST_BINARY_OPERATION:
					accept(dynamic_cast<BinaryOperationAst*>(node));
					break;
				case AstNodeType::AST_UNARY_OPERATION:
					accept(dynamic_cast<UnaryOperationAst*>(node));
					break;
				case AstNodeType::AST_MEMBER_ACCESS:
					accept(dynamic_cast<MemberAccessAst*>(node));
					break;
				case AstNodeType::AST_VARIABLE_DECLARATION:
					accept(dynamic_cast<VariableDeclarationAst*>(node));
					break;
				case AstNodeType::AST_VARIABLE:
					accept(dynamic_cast<VariableAst*>(node));
					break;
				case AstNodeType::AST_INTEGER:
					accept(dynamic_cast<IntegerAst*>(node));
					break;
				case AstNodeType::AST_FLOAT:
					accept(dynamic_cast<FloatAst*>(node));
					break;
				case AstNodeType::AST_STRING:
					accept(dynamic_cast<StringAst*>(node));
					break;
				case AstNodeType::AST_TRUE:
					accept(dynamic_cast<TrueAst*>(node));
					break;
				case AstNodeType::AST_FALSE:
					accept(dynamic_cast<FalseAst*>(node));
					break;
				case AstNodeType::AST_NULL:
					accept(dynamic_cast<NullAst*>(node));
					break;
				case AstNodeType::AST_FUNCTION_DEFINITION:
					accept(dynamic_cast<FunctionDefinitionAst*>(node));
					break;
				case AstNodeType::AST_FUNCTION_CALL:
					accept(dynamic_cast<FunctionCallAst*>(node));
					break;
				case AstNodeType::AST_RETURN_STATEMENT:
					accept(dynamic_cast<ReturnStatementAst*>(node));
					break;
				case AstNodeType::AST_IF_STATEMENT:
					accept(dynamic_cast<IfStatementAst*>(node));
					break;
				case AstNodeType::AST_FOR_LOOP:
					accept(dynamic_cast<ForLoopAst*>(node));
					break;
				default:
					state.errors.push_back({ ErrorType::INTERNAL_ERROR, node->location });
					break;
				}
			}

			void Zen2CppHandler::accept(ImportsAst *node)
			{
				for (auto &child : node->imports)
					accept(child.get());
			}

			void Zen2CppHandler::accept(ImportAst *node)
			{
				if (level != -1)
					state.errors.push_back({ ErrorType::IMPORT_OUTSIDE_GLOBAL, node->location });

				if (node->isModuleImport)
				{
					// TODO: Import module
				}
				else
				{
					// load relative file
					auto importModulePath = node->localPath + node->value;

					// Check if the module has already been imported
					if (externalModules.find(importModulePath) == externalModules.end())
					{
						std::ifstream t(importModulePath);
						if (!t.is_open())
							state.errors.push_back({ ErrorType::MODULE_NOT_FOUND, node->location, node->value });
						else
						{
							std::string str((std::istreambuf_iterator<char>(t)),
								std::istreambuf_iterator<char>());

							t.close();

							Lexer lexer(str, importModulePath);
							auto tokens = lexer.scan();

							Parser parser(tokens, lexer.state);
							auto unit = parser.parse();

							externalModules[importModulePath] = std::move(unit);

							for (auto &&error : parser.state.errors)
								state.errors.push_back(error);

							for (std::unique_ptr<AstNode> &child : externalModules[importModulePath]->children)
								accept(child.get());
						}
					}
				}
			}

			void Zen2CppHandler::accept(StatementAst *node)
			{
			}

			void Zen2CppHandler::accept(BlockAst *node)
			{
				for (int i = 0; i < node->children.size(); i++)
				{
					indent();

					if (node->children[i])
					{
						accept(node->children[i].get());

						append(";\n");
					}
				}
			}

			void Zen2CppHandler::accept(ExpressionAst *node)
			{
				accept(node->value.get());
			}

			void Zen2CppHandler::accept(BinaryOperationAst *node)
			{
				Operator op = node->op;
				auto &left = node->left;
				auto &right = node->right;

				accept(left.get());

				switch (op)
				{
				case Operator::OP_MULTIPLY:
					append("*");
					break;
				case Operator::OP_INT_DIVIDE:
				case Operator::OP_DIVIDE:
					append("/");
					break;
				case Operator::OP_MODULUS:
					append("%");
					break;
				case Operator::OP_ADD:
					append("+");
					break;
				case Operator::OP_SUBTRACT:
				  append("-");
					break;
					/*case Operator::OP_BINARY_XOR:
					addCommand<OpBinaryBitXor>();
					break;
					case Operator::OP_BINARY_AND:
					addCommand<OpBinaryBitAnd>();
					break;
					case Operator::OP_BINARY_OR:
					addCommand<OpBinaryBitOr>();
					break;*/
				case Operator::OP_AND:
					append("&&");
					break;
				case Operator::OP_OR:
					append("||");
					break;
				case Operator::OP_EQUALS:
					append("==");
					break;
				case Operator::OP_NOT_EQUAL:
					append("!=");
					break;
					/*case Operator::OP_OUTPUT:
					addCommand<OpBinaryOut>();
					break;
					case Operator::OP_INPUT:
					addCommand<OpBinaryIn>();
					break;*/
				case Operator::OP_LESS:
					append("<");
					break;
				case Operator::OP_GREATER:
					append(">");
					break;
				case Operator::OP_GREATER_OR_EQUAL:
					append(">=");
					break;
				case Operator::OP_LESS_OR_EQUAL:
					append("<=");
					break;
				case Operator::OP_ASSIGN:
					append("=");
					break;
				case Operator::OP_ADD_ASSIGN:
					append("+=");
					break;
				case Operator::OP_SUBTRACT_ASSIGN:
					append("-=");
					break;
				case Operator::OP_MULTIPLY_ASSIGN:
					append("*=");
					break;
				case Operator::OP_DIVIDE_ASSIGN:
					append("/=");
					break;
					/*case Operator::OP_MODULUS_ASSIGN:
					addCommand<OpBinaryModAssign>();
					break;
					case Operator::OP_BINARY_AND_ASSIGN:
					addCommand<OpBinaryBitAndAssign>();
					break;
					case Operator::OP_BINARY_XOR_ASSIGN:
					addCommand<OpBinaryBitXorAssign>();
					break;
					case Operator::OP_BINARY_OR_ASSIGN:
					addCommand<OpBinaryBitOrAssign>();
					break;*/
				default:
					state.errors.push_back(Error(ErrorType::ILLEGAL_OPERATOR, node->location, getOperatorStr(op)));
					break;
				}
				accept(right.get());
			}

			void Zen2CppHandler::accept(UnaryOperationAst *node)
			{
				Operator op = node->op;
				auto &val = node->value;

				accept(val.get());

				switch (op)
				{
				case Operator::OP_NOT:
					append("!");
					break;
				case Operator::OP_ADD:
					append("+");
					break;
				case Operator::OP_SUBTRACT:
					append("-");
					break;
				default:
					state.errors.push_back(Error(ErrorType::ILLEGAL_OPERATOR, node->location, getOperatorStr(op)));
					break;
				}
			}

			void Zen2CppHandler::accept(MemberAccessAst *node)
			{

			}

			void Zen2CppHandler::accept(VariableDeclarationAst *node)
			{
				Level &currentLevel = levels[level];

				if (std::find(currentLevel.variableNames.begin(), currentLevel.variableNames.end(), node->name)
					!= currentLevel.variableNames.end())
				{
					state.errors.push_back(Error(ErrorType::REDECLARED_IDENTIFIER,
						node->location,
						node->name));
				}
				else
				{
					currentLevel.variableNames.push_back(node->name);
					append(std::string("Any ") + node->name);
				}
			}

			void Zen2CppHandler::accept(VariableAst *node)
			{
				if (!varInScope(node->name))
					state.errors.push_back(Error(ErrorType::UNDECLARED_IDENTIFIER, node->location, node->name));
				else
					append(node->name);
			}

			void Zen2CppHandler::accept(IntegerAst *node)
			{
				append(node->value);
			}

			void Zen2CppHandler::accept(FloatAst *node)
			{
				append(node->value);
			}

			void Zen2CppHandler::accept(StringAst *node)
			{
				append(std::string("std::string(\"") + node->value + "\")");
			}

			void Zen2CppHandler::accept(TrueAst *node)
			{
				append("true");
			}

			void Zen2CppHandler::accept(FalseAst *node)
			{
				append("false");
			}

			void Zen2CppHandler::accept(NullAst *node)
			{
				append("nullptr");
			}

			void Zen2CppHandler::accept(FunctionDefinitionAst *node)
			{

			}

			void Zen2CppHandler::accept(FunctionCallAst *node)
			{

			}

			void Zen2CppHandler::accept(IfStatementAst *node)
			{
				indent();
				append("if (");
				accept(node->cond_expr.get());
				append(")\n");

				increaseBlock(BlockType::IF_STATEMENT_BLOCK);
				accept(node->block.get());
				decreaseBlock();

				if (node->elseStatement != nullptr)
				{
					indent();
					append("else\n");
					increaseBlock(BlockType::ELSE_STATEMENT_BLOCK);
					accept(node->elseStatement.get());
					decreaseBlock();
				}
			}

			void Zen2CppHandler::accept(ReturnStatementAst *node)
			{

			}

			void Zen2CppHandler::accept(ForLoopAst *node)
			{

			}

			bool Zen2CppHandler::varInScope(const std::string &name)
			{
				int startLevel = level;

				while (startLevel >= -1)
				{
					Level &currentLevel = levels.at(startLevel);

					if (std::find(currentLevel.variableNames.begin(),
						currentLevel.variableNames.end(),
						name)
						!= currentLevel.variableNames.end())
					{
						return true;
					}

					startLevel--;
				}

				return false;
			}

			ReturnMessage Zen2CppHandler::fnInScope(const std::string &name, int nArgs, FunctionDefinitionAst *&out)
			{
				int startLevel = level;
				ReturnMessage status = FN_NOT_FOUND;

				while (startLevel >= -1)
				{
					Level &currentLevel = levels.at(startLevel);

					for (auto &&def : currentLevel.functionDeclarations)
					{
						if (def.first == name)
						{
							if (def.second->arguments.size() == nArgs)
							{
								out = def.second;
								return FN_FOUND;
							}
							else if (def.second->arguments.size() < nArgs)
								status = FN_TOO_MANY_ARGS;
							else if (def.second->arguments.size() > nArgs)
								status = FN_TOO_FEW_ARGS;
						}
					}

					startLevel--;
				}

				return status;
			}

			void Zen2CppHandler::increaseBlock(BlockType type)
			{
				indent();
				append("{\n");

				Level frame;
				frame.type = type;
				levels[++level] = frame;
			}

			void Zen2CppHandler::decreaseBlock()
			{
				levels[level--] = Level();

				indent();
				append("}\n");
			}
		}
	}
}
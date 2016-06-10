#include "compiler2.h"

#include <fstream>
using std::ifstream;

#include "../lexer.h"
#include "../parser.h"

namespace zenith
{
	namespace compiler
	{
		void CompilerState::increaseBlock(BlockType type)
		{
			ScopeRepr scopeRepr;
			scopeRepr.type = type;
			currentModule->scopes[++level] = scopeRepr;

			addCommand<IncreaseBlockLevel>();
		}
		
		void CompilerState::decreaseBlock()
		{
			currentModule->scopes[level--] = ScopeRepr();
			addCommand<DecreaseBlockLevel>();
		}

		bool CompilerState::moduleImported(const string &moduleName)
		{
			return std::find(moduleFilepaths.begin(), moduleFilepaths.end(), moduleName) != moduleFilepaths.end();
		}

		void CompilerState::newObject(string &name)
		{
			newObject(name, currentModule);
		}

		void CompilerState::newObject(string &name, ModuleRepr *module)
		{
			ScopeRepr &currentScope = module->scopes[level];

			currentScope.objects.insert({ name, ObjectRepr() });
		}

		void CompilerState::newClassType(string &name)
		{
			newClassType(name, currentModule);
		}

		void CompilerState::newClassType(string &name, ModuleRepr *module)
		{
			module->classTypes.insert({ name, ClassTypeRepr() });
		}

		ModuleRepr::ModuleRepr()
		{
			// initialize global scope
			scopes[-1] = ScopeRepr();
		}

		Compiler::Compiler(ParserState &parserState)
		{
			for (auto &&error : parserState.errors)
				state.errors.push_back(error);
		}

		Compiler::~Compiler()
		{

		}

		void Compiler::accept(ModuleAst *node)
		{
			state.modules.insert({ node->moduleName, ModuleRepr() });
			state.currentModule = &state.modules[node->moduleName];

			for (int i = 0; i < node->children.size(); i++)
				accept(node->children[i].get());
		}

		void Compiler::accept(AstNode *node)
		{
			if (!node)
				return;

			switch (node->nodeType)
			{
			case AST_IMPORTS:
				accept(dynamic_cast<ImportsAst*>(node));
				break;
			case AST_IMPORT:
				accept(dynamic_cast<ImportAst*>(node));
				break;
			case AST_STATEMENT:
				accept(dynamic_cast<StatementAst*>(node));
				break;
			case AST_BLOCK:
				accept(dynamic_cast<BlockAst*>(node));
				break;
			case AST_EXPRESSION:
				accept(dynamic_cast<ExpressionAst*>(node));
				break;
			case AST_BINARY_OPERATION:
				accept(dynamic_cast<BinaryOperationAst*>(node));
				break;
			case AST_UNARY_OPERATION:
				accept(dynamic_cast<UnaryOperationAst*>(node));
				break;
			case AST_MEMBER_ACCESS:
				accept(dynamic_cast<MemberAccessAst*>(node));
				break;
			case AST_VARIABLE_DECLARATION:
				accept(dynamic_cast<VariableDeclarationAst*>(node));
				break;
			case AST_VARIABLE:
				accept(dynamic_cast<VariableAst*>(node));
				break;
			case AST_INTEGER:
				accept(dynamic_cast<IntegerAst*>(node));
				break;
			case AST_FLOAT:
				accept(dynamic_cast<FloatAst*>(node));
				break;
			case AST_STRING:
				accept(dynamic_cast<StringAst*>(node));
				break;
			case AST_TRUE:
				accept(dynamic_cast<TrueAst*>(node));
				break;
			case AST_FALSE:
				accept(dynamic_cast<FalseAst*>(node));
				break;
			case AST_NULL:
				accept(dynamic_cast<NullAst*>(node));
				break;
			case AST_SELF:
				accept(dynamic_cast<SelfAst*>(node));
				break;
			case AST_NEW:
				accept(dynamic_cast<NewAst*>(node));
				break;
			case AST_FUNCTION_DEFINITION:
				accept(dynamic_cast<FunctionDefinitionAst*>(node));
				break;
			case AST_FUNCTION_CALL:
				accept(dynamic_cast<FunctionCallAst*>(node));
				break;
			case AST_CLASS:
				accept(dynamic_cast<ClassAst*>(node));
				break;
			case AST_RETURN_STATEMENT:
				accept(dynamic_cast<ReturnStatementAst*>(node));
				break;
			case AST_IF_STATEMENT:
				accept(dynamic_cast<IfStatementAst*>(node));
				break;
			case AST_FOR_LOOP:
				accept(dynamic_cast<ForLoopAst*>(node));
				break;
			default:
				state.errors.push_back({ INTERNAL_ERROR, node->location });
				break;
			}
		}

		void Compiler::accept(ImportsAst *node)
		{
			for (auto &child : node->imports)
				accept(child.get());
		}

		void Compiler::accept(ImportAst *node)
		{
			if (state.level != -1)
				state.errors.push_back({ IMPORT_OUTSIDE_GLOBAL, node->location });

			if (node->isModuleImport)
			{
				// TODO: Import module
			}
			else
			{
				// load relative file
				auto importModulePath = node->localPath + node->value;

				// Check if the module has already been imported
				if (!state.moduleImported(importModulePath))
				{
					ifstream t(importModulePath);

					if (!t.is_open())
						state.errors.push_back({ MODULE_NOT_FOUND, node->location, node->value });
					else
					{
						string str((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());

						t.close();

						Lexer lexer(str, importModulePath);
						auto tokens = lexer.scan();

						Parser parser(tokens, lexer.state);
						auto unit = parser.parse();

						if (state.modules.find(unit->moduleName) == state.modules.end())
						{
							state.moduleFilepaths.push_back(importModulePath);
							state.modules.insert({unit->moduleName, ModuleRepr()});

							ModuleRepr *moduleBefore = state.currentModule;
							state.currentModule = &state.modules[unit->moduleName];

							for (auto &&error : parser.state.errors)
								state.errors.push_back(error);

							for (std::unique_ptr<AstNode> &child : unit->children)
								accept(child.get());

							state.currentModule = moduleBefore;
						}
						else // module's identifier has already been declared
							state.errors.push_back({ MODULE_ALREADY_DEFINED, node->location, unit->moduleName });
					}
				}
			}
		}

		void Compiler::accept(StatementAst *node)
		{

		}

		void Compiler::accept(BlockAst *node)
		{
			for (auto &&child : node->children)
				accept(child.get());
		}

		void Compiler::accept(ExpressionAst *node)
		{
			accept(node->value.get());

			if (node->shouldClearStack)
				state.addCommand<OpClear>();
		}

		void Compiler::accept(BinaryOperationAst *node)
		{
			auto &left = node->left;
			auto &right = node->right;

			accept(left.get());
			accept(right.get());

			switch (node->op)
			{
			case OP_POWER:
				state.addCommand<OpBinaryPow>();
				break;
			case OP_MULTIPLY:
				state.addCommand<OpBinaryMul>();
				break;
			case OP_INT_DIVIDE:
			case OP_DIVIDE:
				state.addCommand<OpBinaryDiv>();
				break;
			case OP_MODULUS:
				state.addCommand<OpBinaryMod>();
				break;
			case OP_ADD:
				state.addCommand<OpBinaryAdd>();
				break;
			case OP_SUBTRACT:
				state.addCommand<OpBinarySub>();
				break;
				/*case OP_BINARY_XOR:
				addCommand<OpBinaryBitXor>();
				break;
				case OP_BINARY_AND:
				addCommand<OpBinaryBitAnd>();
				break;
				case OP_BINARY_OR:
				addCommand<OpBinaryBitOr>();
				break;*/
			case OP_AND:
				state.addCommand<OpBinaryAnd>();
				break;
			case OP_OR:
				state.addCommand<OpBinaryOr>();
				break;
			case OP_EQUALS:
				state.addCommand<OpBinaryEql>();
				break;
			case OP_NOT_EQUAL:
				state.addCommand<OpBinaryNotEql>();
				break;
				/*case OP_OUTPUT:
				state.addCommand<OpBinaryOut>();
				break;
				case OP_INPUT:
				state.addCommand<OpBinaryIn>();
				break;*/
			case OP_LESS:
				state.addCommand<OpBinaryLT>();
				break;
			case OP_GREATER:
				state.addCommand<OpBinaryGT>();
				break;
			case OP_GREATER_OR_EQUAL:
				state.addCommand<OpBinaryGTE>();
				break;
			case OP_LESS_OR_EQUAL:
				state.addCommand<OpBinaryLTE>();
				break;
			case OP_ASSIGN:
			{
				state.addCommand<OpBinaryAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_ADD_ASSIGN:
			{
				state.addCommand<OpBinaryAddAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_SUBTRACT_ASSIGN:
			{
				state.addCommand<OpBinarySubAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_MULTIPLY_ASSIGN:
			{
				state.addCommand<OpBinaryMulAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_DIVIDE_ASSIGN:
			{
				state.addCommand<OpBinaryDivAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			/*case Operator::OP_MODULUS_ASSIGN:
			state.addCommand<OpBinaryModAssign>();
			break;
			case Operator::OP_BINARY_AND_ASSIGN:
			state.addCommand<OpBinaryBitAndAssign>();
			break;
			case Operator::OP_BINARY_XOR_ASSIGN:
			state.addCommand<OpBinaryBitXorAssign>();
			break;
			case Operator::OP_BINARY_OR_ASSIGN:
			state.addCommand<OpBinaryBitOrAssign>();
			break;*/
			default:
				state.errors.push_back({ ILLEGAL_OPERATOR,
					node->location,
					getOperatorStr(node->op) });
				break;
			}
		}

		void Compiler::accept(UnaryOperationAst *node)
		{
			accept(node->value.get());

			switch (node->op)
			{
			case OP_NOT:
				state.addCommand<OpUnaryNot>();
				break;
			case OP_ADD:
				state.addCommand<OpUnaryPos>();
				break;
			case OP_SUBTRACT:
				state.addCommand<OpUnaryMinus>();
				break;
			default:
				state.errors.push_back({ ILLEGAL_OPERATOR,
					node->location,
					getOperatorStr(node->op) });
				break;
			}
		}

		void Compiler::accept(MemberAccessAst *node)
		{

		}

		void Compiler::accept(VariableDeclarationAst *node)
		{
			if (Util::legalIdentifier(state, node->name))
			{
				state.newObject(node->name);
				state.addCommand<VarCreate>(VAR_TYPE_ANY, node->name);

				if (node->assignment != nullptr)
					accept(node->assignment.get());
			}
			else
				state.errors.push_back(Error(REDECLARED_IDENTIFIER,
					node->location,
					node->name));
		}

		void Compiler::accept(VariableAst *node)
		{
			if (Util::findVariable(state, node->name))
				state.addCommand<LoadVariable>(node->name);
			else
				state.errors.push_back({ UNDECLARED_IDENTIFIER,
					node->location,
					node->name });
		}

		void Compiler::accept(IntegerAst *node)
		{
			state.addCommand<LoadInteger>(node->value);
		}

		void Compiler::accept(FloatAst *node)
		{
			state.addCommand<LoadFloat>(node->value);
		}

		void Compiler::accept(StringAst *node)
		{
			state.addCommand<LoadString>(node->value);
		}

		void Compiler::accept(TrueAst *node)
		{
			state.addCommand<LoadInteger>(1);
		}

		void Compiler::accept(FalseAst *node)
		{
			state.addCommand<LoadInteger>(0);
		}

		void Compiler::accept(NullAst *node)
		{
			state.addCommand<LoadNull>();
		}

		void Compiler::accept(SelfAst *node)
		{
			// TODO
		}

		void Compiler::accept(NewAst *node)
		{

		}

		void Compiler::accept(FunctionDefinitionAst *node)
		{
			if (!Util::legalIdentifier(state, node->name))
				state.errors.push_back({ REDECLARED_IDENTIFIER,
					node->location,
					node->name });
		}

		void Compiler::accept(FunctionCallAst *node)
		{

		}

		void Compiler::accept(ClassAst *node)
		{

		}

		void Compiler::accept(IfStatementAst *node)
		{

		}

		void Compiler::accept(ReturnStatementAst *node)
		{

		}

		void Compiler::accept(ForLoopAst *node)
		{

		}

		bool Util::findVariable(CompilerState &state,
			const string &name,
			bool thisScopeOnly)
		{
			return findVariable(state, name, state.level, thisScopeOnly);
		}

		bool Util::findVariable(CompilerState &state,
			const string &name,
			int scope,
			bool thisScopeOnly)
		{
			int search = scope;
			while (search >= -1)
			{
				ScopeRepr &scopeRepr = state.currentModule->scopes[search];

				if (scopeRepr.objects.find(name) != scopeRepr.objects.end())
					return true;

				if (thisScopeOnly)
					break;

				search--;
			}

			return false;
		}

		bool Util::legalIdentifier(CompilerState &state, const string &name)
		{
			if (state.modules.find(name) != state.modules.end())
				return false;

			if (findVariable(state, name, true))
				return false;

			for (auto &&module : state.modules)
			{
				if (module.second.classTypes.find(name) != 
							module.second.classTypes.end())
					return false;
			}

			return true;
		}
	}
}
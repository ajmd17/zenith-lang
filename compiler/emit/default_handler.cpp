#include "default_handler.h"

#include <algorithm>
#include <fstream>

#include "bytecode.h"

#include "../lexer.h"
#include "../parser.h"

namespace zenith
{
	namespace compiler
	{
		const std::string DefaultAstHandler::SELF_DEFAULT = "$_self_default"; // default self, null
		const std::string DefaultAstHandler::SELF_GLOBAL = "$_self_global"; // used to access outside members from within a class
		const int DefaultAstHandler::LEVEL_GLOBAL = -1; // global variables are on this level

		DefaultAstHandler::DefaultAstHandler(ParserState &state)
		{
			this->state = state;
			this->self = { SELF_DEFAULT, nullptr };
			levels[level] = Level();
		}

		DefaultAstHandler::~DefaultAstHandler()
		{
			functionDefBlockIds.clear();
			nativeFunctions.clear();
			externalModules.clear();
		}

		void DefaultAstHandler::accept(ModuleAst *node)
		{
			mainModule = node;

			for (int i = 0; i < node->children.size(); i++)
				accept(node->children[i].get());
		}

		void DefaultAstHandler::accept(AstNode *node)
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

		void DefaultAstHandler::accept(ImportsAst *node)
		{
			for (auto &child : node->imports)
				accept(child.get());
		}

		void DefaultAstHandler::accept(ImportAst *node)
		{
			if (level != LEVEL_GLOBAL)
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
				if (externalModules.find(importModulePath) == externalModules.end())
				{
					std::ifstream t(importModulePath);

					if (!t.is_open())
						state.errors.push_back({ MODULE_NOT_FOUND, node->location, node->value });
					else
					{
						std::string str((std::istreambuf_iterator<char>(t)),
							std::istreambuf_iterator<char>());

						t.close();

						Lexer lexer(str, importModulePath);
						auto tokens = lexer.scan();

						Parser parser(tokens, lexer.state);
						auto unit = parser.parse();

						if (!isIdentifier(unit->moduleName))
						{
							externalModules[importModulePath] = std::move(unit);

							for (auto &&error : parser.state.errors)
								state.errors.push_back(error);

							for (std::unique_ptr<AstNode> &child : externalModules[importModulePath]->children)
								accept(child.get());
						}
						else // module's identifier has already been declared
							state.errors.push_back({ MODULE_ALREADY_DEFINED, node->location, unit->moduleName });
					}
				}
			}
		}

		void DefaultAstHandler::accept(StatementAst *node)
		{
		}

		void DefaultAstHandler::accept(BlockAst *node)
		{
			for (auto &&child : node->children)
				accept(child.get());
		}

		void DefaultAstHandler::accept(ExpressionAst *node)
		{
			accept(node->value.get());

			if (node->shouldClearStack)
				addCommand<OpClear>();
		}

		void DefaultAstHandler::accept(BinaryOperationAst *node)
		{
			auto &left = node->left;
			auto &right = node->right;

			accept(left.get());
			accept(right.get());

			switch (node->op)
			{
			case OP_POWER:
				addCommand<OpBinaryPow>();
				break;
			case OP_MULTIPLY:
				addCommand<OpBinaryMul>();
				break;
			case OP_INT_DIVIDE:
			case OP_DIVIDE:
				addCommand<OpBinaryDiv>();
				break;
			case OP_MODULUS:
				addCommand<OpBinaryMod>();
				break;
			case OP_ADD:
				addCommand<OpBinaryAdd>();
				break;
			case OP_SUBTRACT:
				addCommand<OpBinarySub>();
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
				addCommand<OpBinaryAnd>();
				break;
			case OP_OR:
				addCommand<OpBinaryOr>();
				break;
			case OP_EQUALS:
				addCommand<OpBinaryEql>();
				break;
			case OP_NOT_EQUAL:
				addCommand<OpBinaryNotEql>();
				break;
				/*case OP_OUTPUT:
				addCommand<OpBinaryOut>();
				break;
				case OP_INPUT:
				addCommand<OpBinaryIn>();
				break;*/
			case OP_LESS:
				addCommand<OpBinaryLT>();
				break;
			case OP_GREATER:
				addCommand<OpBinaryGT>();
				break;
			case OP_GREATER_OR_EQUAL:
				addCommand<OpBinaryGTE>();
				break;
			case OP_LESS_OR_EQUAL:
				addCommand<OpBinaryLTE>();
				break;
			case OP_ASSIGN:
			{
				addCommand<OpBinaryAssign>();

				if ((left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
				{
				}
				else
				{
					// cannot assign a value to a number or string, etc.
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });
				}

				break;
			}
			case OP_ADD_ASSIGN:
			{
				addCommand<OpBinaryAddAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_SUBTRACT_ASSIGN:
			{
				addCommand<OpBinarySubAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_MULTIPLY_ASSIGN:
			{
				addCommand<OpBinaryMulAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
			case OP_DIVIDE_ASSIGN:
			{
				addCommand<OpBinaryDivAssign>();

				// cannot assign a value to a number or string, etc.
				if (!(left->nodeType == AST_VARIABLE ||
					left->nodeType == AST_MEMBER_ACCESS))
					state.errors.push_back({ ILLEGAL_EXPRESSION, left->location });

				break;
			}
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
				state.errors.push_back({ ILLEGAL_OPERATOR,
					node->location,
					getOperatorStr(node->op) });
				break;
			}
		}

		void DefaultAstHandler::accept(UnaryOperationAst *node)
		{
			accept(node->value.get());

			switch (node->op)
			{
			case OP_NOT:
				addCommand<OpUnaryNot>();
				break;
			case OP_ADD:
				addCommand<OpUnaryPos>();
				break;
			case OP_SUBTRACT:
				addCommand<OpUnaryMinus>();
				break;
			default:
				state.errors.push_back({ ILLEGAL_OPERATOR,
					node->location,
					getOperatorStr(node->op) });
				break;
			}
		}

		void DefaultAstHandler::accept(MemberAccessAst *node)
		{
			auto *current = loopMemberAccess(node);

			if (current)
			{
				switch (current->nodeType)
				{
				case AST_VARIABLE:
				case AST_FUNCTION_CALL:
					accept(current);
					return;
				}
			}

			state.errors.push_back({ EXPECTED_IDENTIFIER, node->location });
		}

		void DefaultAstHandler::accept(VariableDeclarationAst *node)
		{
			std::string identName = makeIdentifier(node->module,
				node->self,
				node->name);

			if (isIdentifier(identName, true) || isModule(node->name))
			{
				state.errors.push_back(Error(REDECLARED_IDENTIFIER,
					node->location,
					node->name));
			}
			else
			{
				levels[level].variableNames.insert({ identName, {false, nullptr} });
				addCommand<VarCreate>(VAR_TYPE_ANY, identName);

				if (node->assignment != nullptr)
					accept(node->assignment.get());
			}
		}

		void DefaultAstHandler::accept(VariableAst *node)
		{
			std::string identName = makeIdentifier(node->module, node->self, node->name);

			if (!varInScope(identName))
				state.errors.push_back({ UNDECLARED_IDENTIFIER,
					node->location,
					node->name });
			else
				addCommand<LoadVariable>(identName);
		}

		void DefaultAstHandler::accept(IntegerAst *node)
		{
			addCommand<LoadInteger>(node->value);
		}

		void DefaultAstHandler::accept(FloatAst *node)
		{
			addCommand<LoadFloat>(node->value);
		}

		void DefaultAstHandler::accept(StringAst *node)
		{
			addCommand<LoadString>(node->value);
		}

		void DefaultAstHandler::accept(TrueAst *node)
		{
			addCommand<LoadInteger>(1);
		}

		void DefaultAstHandler::accept(FalseAst *node)
		{
			addCommand<LoadInteger>(0);
		}

		void DefaultAstHandler::accept(NullAst *node)
		{
			addCommand<LoadNull>();
		}

		void DefaultAstHandler::accept(SelfAst *node)
		{
			if (self.second == nullptr)
				state.errors.push_back({ SELF_NOT_DEFINED, node->location });

			else
			{
				// load the variable that "self" refers to
				std::string &selfName = self.first;

				if (!varInScope(selfName))
					state.errors.push_back({ SELF_NOT_DEFINED, node->location });
				else
					addCommand<LoadVariable>(selfName);
			}
		}

		void DefaultAstHandler::accept(NewAst *node)
		{
			auto *constructor = node->constructorAst.get();
			if (constructor != nullptr)
			{
				if (constructor->nodeType == AST_MEMBER_ACCESS)
					constructor = loopMemberAccess(dynamic_cast<MemberAccessAst*>(constructor));

				auto *constructorAst = dynamic_cast<FunctionCallAst*>(constructor);
				// TODO: find correct constructor
				if (constructorAst != nullptr)
				{
					std::string classType = constructorAst->name;

					if (classInstanceId.find(classType) == classInstanceId.end())
						classInstanceId[classType] = 1;
					else
						classInstanceId[classType] += 1;

					std::string mangledClassName = makeIdentifier(constructorAst->module,
						constructorAst->self,
						classType);

					bool isAnonClass = node->identifier.empty();

					std::string mangledClassInstance = makeIdentifier(node->module,
						node->self,
						(!isAnonClass) ?
						node->identifier :
						(mangledClassName + std::to_string(classInstanceId[classType])));

					auto it = classTypes.find(mangledClassName);
					if (it != classTypes.end())
					{
						Level &currentLevel = levels[level];

						if (!isAnonClass)
						{
							// get original variable level
							auto varLevel = getVarLevel(mangledClassInstance); // the variable should already be created
							if (varLevel < LEVEL_GLOBAL)
							{
								state.errors.push_back({ UNDECLARED_IDENTIFIER,
									node->location,
									mangledClassInstance });
							}
							else
							{
								// modify to show that it is now a class object
								// so we can get and set properties on it
								auto &varInfo = levels[varLevel].variableNames[mangledClassInstance];

								varInfo.isClass = true;
								varInfo.classType = it->second;
							}
						}
						else
							currentLevel.variableNames.insert({ mangledClassInstance, {true, it->second} });

						// TODO: Load an actual wrapper to what class type this is
						addCommand<LoadString>(classType);

						// create class instance
						self = { mangledClassInstance, it->second };
						for (auto &&member : it->second->dataMembers)
						{
							if (member->nodeType == AST_VARIABLE_DECLARATION)
							{
								auto *varDecl = dynamic_cast<VariableDeclarationAst*>(member.get());
								// testing adding object member
								addCommand<AddMember>(varDecl->name);
							}

							accept(member.get());
						}
						self = { SELF_DEFAULT, nullptr };
					}
					else
						state.errors.push_back({ UNKNOWN_CLASS_TYPE, node->location, classType });
				}
				else
					state.errors.push_back({ INVALID_CONSTRUCTOR, node->location });
			}
		}

		void DefaultAstHandler::accept(FunctionDefinitionAst *node)
		{
			std::string mangledName = makeIdentifier(node->module, node->self, node->name, node->arguments.size());

			FunctionDefinitionAst *tmpNode = nullptr;
			if ((fnInScope(mangledName, node->arguments.size(), tmpNode) == FN_FOUND) ||
				varInScope(mangledName) ||
				isModule(node->name))
			{
				state.errors.push_back({ REDECLARED_IDENTIFIER,
					node->location,
					node->name });
			}
			else
			{
				levels[level].functionDeclarations.push_back({ mangledName, node });

			/*	functionDefBlockIds[node] = blockIdNum;
				addCommand<CreateBlock>(FUNCTION_BLOCK,
					blockIdNum++,
					level);*/

				addCommand<CreateFunction>(mangledName);

				auto *fnBody = dynamic_cast<BlockAst*>(node->block.get());

				if (fnBody != nullptr)
				{
					// add return statement
					if (fnBody->children.size() == 0 ||
						fnBody->children.back()->nodeType != AST_RETURN_STATEMENT)
					{
						auto returnValue = std::make_unique<NullAst>(node->block->location, node->module);
						auto returnStatement = std::make_unique<ReturnStatementAst>(node->block->location,
							node->module,
							std::move(returnValue));
						fnBody->addChild(std::move(returnStatement));
					}

					increaseBlock(FUNCTION_BLOCK);

					for (int i = node->arguments.size() - 1; i >= 0; i--)
					{
						const std::string &str = node->arguments[i];
						// mangle argument variable
						std::string mangledArg = makeIdentifier(node->module, node->self, str);

						levels[level].variableNames.insert({ mangledArg, {false, nullptr} });

						addCommand<VarCreate>(VAR_TYPE_ANY, mangledArg);
						addCommand<StackPopObject>(mangledArg, STACK_FUNCTION_PARAM);
					}

					accept(fnBody);
					decreaseBlock();
				}
			}
		}

		void DefaultAstHandler::accept(FunctionCallAst *node)
		{
			FunctionDefinitionAst *definition = nullptr;
			std::string mangledName = makeIdentifier(node->module, node->self, node->name, node->arguments.size());
			ReturnMessage msg = fnInScope(mangledName, node->arguments.size(), definition);

			if (msg == FN_NOT_FOUND)
				state.errors.push_back({ FUNCTION_NOT_FOUND, node->location, node->name + " (" + unmangleIdentifier(mangledName) + ")" });
			else if (msg == FN_TOO_MANY_ARGS)
				state.errors.push_back({ TOO_MANY_ARGS, node->location, node->name });
			else if (msg == FN_TOO_FEW_ARGS)
				state.errors.push_back({ TOO_FEW_ARGS, node->location, node->name });
			else if (msg == FN_FOUND)
			{
				// create in reverse order
				for (int i = node->arguments.size() - 1; i >= 0; i--)
				{
					// must temporarily increase block level to avoid conflicts
					addCommand<IncreaseReadLevel>();
					increaseBlock(UNDEFINED_BLOCK);

					accept(node->arguments[i].get());
					addCommand<OpPush>(STACK_FUNCTION_PARAM);

					decreaseBlock();
				}

				// Call the function. The variable name is passed so that it can be set to the result
				if (!definition->isNative)
				{
					addCommand<LoadVariable>(mangledName);
					addCommand<Invoke>();
				}
				else
					addCommand<CallNativeFunction>(node->name,
						functionDefBlockIds[definition],
						definition->arguments.size());
			}
		}

		void DefaultAstHandler::accept(ClassAst *node)
		{
			std::string mangledName = makeIdentifier(node->module, 
				node->self, 
				node->name);

			if (isIdentifier(mangledName) || isModule(node->name))
			{
				state.errors.push_back(Error(REDECLARED_IDENTIFIER,
					node->location,
					node->name));
			}
			else
				// add the class type for later use
				classTypes.insert({ mangledName, node });
		}

		void DefaultAstHandler::accept(IfStatementAst *node)
		{
			accept(node->cond_expr.get());

			addCommand<IfStatement>();
			addCommand<CreateBlock>(IF_STATEMENT_BLOCK,
				blockIdNum++,
				level);

			increaseBlock(IF_STATEMENT_BLOCK);
			accept(node->block.get());
			decreaseBlock();

			if (node->elseStatement != nullptr)
			{
				addCommand<ElseStatement>();
				increaseBlock(ELSE_STATEMENT_BLOCK);
				accept(node->elseStatement.get());
				decreaseBlock();
			}
		}

		void DefaultAstHandler::accept(ReturnStatementAst *node)
		{
			accept(node->value.get());
			addCommand<OpPush>(STACK_FUNCTION_CALLBACK);

			int startLevel = level;
			Level *tmpFrame = &levels[startLevel];

			while (startLevel >= LEVEL_GLOBAL &&
				tmpFrame->type != FUNCTION_BLOCK)
			{
				//TODO: clear vars from block
				addCommand<LeaveBlock>();

				tmpFrame = &levels[--startLevel];
			}
			//TODO: clear vars from final block
			addCommand<LeaveFunction>();
		}

		void DefaultAstHandler::accept(ForLoopAst *node)
		{
			// temporarily increase block level to avoid conflicts
			addCommand<IncreaseReadLevel>();
			increaseBlock(UNDEFINED_BLOCK);

			if (node->init_expr != nullptr)
				accept(node->init_expr.get());

			addCommand<CreateBlock>(LABEL_BLOCK,
				blockIdNum++,
				level);

			accept(node->cond_expr.get());
			addCommand<IfStatement>();

			increaseBlock(IF_STATEMENT_BLOCK);
			accept(node->block.get());

			if (node->inc_expr != nullptr)
				accept(node->inc_expr.get());

			decreaseBlock();
			addCommand<GoToIfTrue>(blockIdNum - 1);
			decreaseBlock();
		}

		// this function is recursive and will return the tail member access
		AstNode *DefaultAstHandler::loopMemberAccess(MemberAccessAst *node)
		{
			auto *defaultModule = dynamic_cast<ModuleAst*>(node->module);

			std::pair<std::string, ClassAst*> defaultSelf = self;
			if (node->self.second)
				defaultSelf = node->self;

			std::string mangledName;
			bool isModuleName = false, 
				isSelfRef = false, 
				isVariableName = false, 
				isFunctionName = false;

			switch (node->left->nodeType)
			{
			case AST_VARIABLE:
			{
				// doesn't have to be a variable, could be module, class etc.
				auto *leftAst = dynamic_cast<VariableAst*>(node->left.get());

				// Check if it is a module
				if (leftAst->name == defaultModule->moduleName)
				{
					// TODO: maybe add a warning that it is
					// redundant to mention module name?
					isModuleName = true;
					defaultSelf = { SELF_GLOBAL, nullptr };
				}
				else
				{
					for (auto &&module : externalModules)
					{
						if (leftAst->name == module.second->moduleName)
						{
							isModuleName = true;
							defaultModule = module.second.get();
							defaultSelf = { SELF_GLOBAL, nullptr };

							break;
						}
					}
				}

				// search for variable
				mangledName = makeIdentifier(defaultModule,
					defaultSelf,
					leftAst->name);

				VariableInfo varInfo;
				if (varInScope(mangledName, varInfo))
				{
					isVariableName = true;

					defaultSelf = { mangledName, varInfo.classType };
					if (varInfo.isClass)
					{
						defaultModule = dynamic_cast<ModuleAst*>(varInfo.classType->module);
					}
				}
				break;
			}
			case AST_FUNCTION_CALL:
			{
				auto *leftAst = dynamic_cast<FunctionCallAst*>(node->left.get());
				mangledName = makeIdentifier(defaultModule,
					defaultSelf,
					leftAst->name);

				FunctionDefinitionAst *tmpFn = nullptr;
				if (fnInScope(mangledName, 0, tmpFn) != ReturnMessage::FN_NOT_FOUND)
				{
					isFunctionName = true;

					defaultSelf = { mangledName, nullptr };
				}
				break;
			}
			case AST_SELF:
			{
				auto *leftAst = dynamic_cast<SelfAst*>(node->left.get());
				// self not defined in this context
				if (!self.second)
					state.errors.push_back({ SELF_NOT_DEFINED, leftAst->location });

				defaultSelf = self;
				isSelfRef = true;
				break;
			}
			default:
				isModuleName = false;
				isSelfRef = false;
				isVariableName = false;
				isFunctionName = false;
				break;
			}

			if (!(isModuleName ||
				isSelfRef ||
				isVariableName ||
				isFunctionName)) {
				state.errors.push_back({ UNDECLARED_IDENTIFIER, node->location, node->leftStr });
			}
			else if (isModuleName);
			else if (isVariableName);
			else if (isFunctionName);
			else if (isSelfRef);
			else
				// may be any combination
				state.errors.push_back({ AMBIGUOUS_IDENTIFIER, node->location, node->leftStr });

			auto &next = node->right;
			if (!next)
				return nullptr;

			next->module = defaultModule;
			next->self = defaultSelf;

			if (next->nodeType == AST_MEMBER_ACCESS)
			{
				auto *nextAst = dynamic_cast<MemberAccessAst*>(next.get());
				return loopMemberAccess(nextAst);
			}
			else
				// not member access, return final
				return next.get();
		}

		bool DefaultAstHandler::isModule(const std::string &name)
		{
			if (name == mainModule->moduleName)
				return true;

			for (auto &&mod : externalModules)
			{
				if (mod.second->moduleName == name)
					return true;
			}

			return false;
		}

		bool DefaultAstHandler::varInScope(const std::string &name)
		{
			VariableInfo dummyInfo;
			return varInScope(name, dummyInfo);
		}

		bool DefaultAstHandler::varInScope(const std::string &name,
			VariableInfo &outInfo)
		{
			int startLevel = level;

			while (startLevel >= LEVEL_GLOBAL)
			{
				Level &currentLevel = levels.at(startLevel);

				auto it = currentLevel.variableNames.find(name);
				if (it != currentLevel.variableNames.end())
				{
					outInfo = it->second;
					return true;
				}

				startLevel--;
			}

			return false;
		}

		int DefaultAstHandler::getVarLevel(const std::string &name)
		{
			int startLevel = level;

			while (startLevel >= LEVEL_GLOBAL)
			{
				Level &currentLevel = levels.at(startLevel);

				auto it = currentLevel.variableNames.find(name);
				if (it != currentLevel.variableNames.end())
				{
					return startLevel;
				}

				startLevel--;
			}

			return LEVEL_GLOBAL - 1;
		}

		ReturnMessage DefaultAstHandler::fnInScope(const std::string &name, int nArgs, FunctionDefinitionAst *&out)
		{
			ReturnMessage status = FN_NOT_FOUND;

			int startLevel = level;
			while (startLevel >= LEVEL_GLOBAL)
			{
				Level &currentLevel = levels.at(startLevel);

				for (auto &&def : currentLevel.functionDeclarations)
				{
					if (def.first == name)
					{
						if (def.second->arguments.size() < nArgs)
							status = FN_TOO_MANY_ARGS;
						else if (def.second->arguments.size() > nArgs)
							status = FN_TOO_FEW_ARGS;
						else
						{
							out = def.second;
							return FN_FOUND;
						}
					}
				}

				startLevel--;
			}

			return status;
		}

		void DefaultAstHandler::increaseBlock(BlockType type)
		{
			Level frame;
			frame.type = type;
			levels[++level] = frame;

			addCommand<IncreaseBlockLevel>();
		}

		void DefaultAstHandler::decreaseBlock()
		{
			levels[level--] = Level();
			addCommand<DecreaseBlockLevel>();
		}

		std::string DefaultAstHandler::makeIdentifier(AstNode *moduleAst,
			std::pair<std::string, ClassAst*> memberOf,
			const std::string &original,
			int numArguments)
		{
			if (!moduleAst || moduleAst->nodeType != AST_MODULE)
				throw std::runtime_error("Invalid module");

			auto *module = dynamic_cast<ModuleAst*>(moduleAst);
			if (!module)
				throw std::runtime_error("Not a module");

			std::string result("$");
			result += "_M" + module->moduleName;

			std::pair<std::string, ClassAst*> clazz = { "", nullptr };
			if (memberOf.first != SELF_GLOBAL)
			{
				if (!memberOf.second)
					clazz = self;
				else
				{
					clazz = memberOf;
					if (!clazz.second)
						throw std::runtime_error("Not a class");
				}
			}

			if (clazz.second != nullptr)
				result += "_C" + clazz.first;

			result += "_I" + original;

			if (numArguments > 0)
				result += "_A" + std::to_string(numArguments);

			return result;
		}

		std::string DefaultAstHandler::unmangleIdentifier(const std::string &mangled)
		{
			std::string moduleName, className, objectName;

			bool inModule = false, inClass = false, inObject = false;
			size_t position = 0, size = mangled.size();

			std::string *currentString = &moduleName;

			// mangled identifiers start with a $ to prevent collision with other variables
			if (mangled[position++] != '$')
				throw std::exception("Not a mangled identifier");

			char c = mangled[position];
			while (c != '\0')
			{
				if (c == '_' && position < (size - 1))
				{
					char nextChar = mangled[position + 1];

					switch (nextChar)
					{
					case 'M':
						currentString = &moduleName;
						break;
					case 'C':
						currentString = &className;
						break;
					case 'I':
						currentString = &objectName;
						break;
					default:
						(*currentString) += std::string("_") + nextChar;
						break;
					}

					++position;
				}
				else if (c == '$')
				{
				}
				else
					(*currentString) += c;

				c = mangled[++position];
			}

			std::string result;
			if (!moduleName.empty())
				result += moduleName + ".";
			if (!className.empty())
				result += className + ".";
			result += objectName;

			return result;
		}

		// is this identifier already created in an accessible scope?
		bool DefaultAstHandler::isIdentifier(const std::string &name, bool thisScopeOnly)
		{
			if (isModule(name))
				return true;

			if (thisScopeOnly)
			{
				auto &currentLevel = levels[level];

				// search variables
				if (currentLevel.variableNames.find(name)
					!= currentLevel.variableNames.end())
					return true;
				else
				{
					// search functions
					auto elt = std::find_if(currentLevel.functionDeclarations.begin(),
						currentLevel.functionDeclarations.end(),
						[&name](const std::pair<std::string, FunctionDefinitionAst*> &element)
					{
						return element.first == name;
					});

					if (elt != currentLevel.functionDeclarations.end())
						return true;
					else
					{
						// search class types
						if (classTypes.find(name) != classTypes.end())
							return true;
						else
							return false;
					}
				}
			}
			else
			{
				FunctionDefinitionAst *tmpFn = nullptr;

				if (varInScope(name))
					return true;
				else if (fnInScope(name, 0, tmpFn) != ReturnMessage::FN_NOT_FOUND)
					return true;
				else if (classTypes.find(name) != classTypes.end())
					return true;
				else
					return false;
			}
		}

		void DefaultAstHandler::defineFunction(const std::string &name,
			const std::string &moduleName,
			size_t numArgs)
		{
			std::vector<std::string> args;
			for (size_t i = 0; i < numArgs; i++)
			{
				std::string arg = "$_M" + moduleName + "_I" + name + "_arg" + std::to_string(i);
				args.push_back(arg);
			}

			std::string mangledName = "$_M" + moduleName + "_I" + name;

			if (numArgs > 0)
				mangledName += "_A" + std::to_string(numArgs);

			std::unique_ptr<FunctionDefinitionAst> definition
				= std::make_unique<FunctionDefinitionAst>(SourceLocation(-1, -1, ""),
					nullptr,
					mangledName,
					args,
					nullptr,
					true);

			nativeFunctions.push_back(std::move(definition));
			levels[level].functionDeclarations.push_back({ mangledName, nativeFunctions.back().get() });

			functionDefBlockIds[nativeFunctions.back().get()] = blockIdNum;
			addCommand<CreateBlock>(FUNCTION_BLOCK,
				blockIdNum++,
				level);
		}
	}
}
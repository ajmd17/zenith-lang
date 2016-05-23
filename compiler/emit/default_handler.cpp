#include "default_handler.h"

#include <algorithm>
#include <fstream>

#include "../lexer.h"
#include "../parser.h"

namespace zenith
{
	DefaultAstHandler::DefaultAstHandler(ParserState &state)
	{
		this->state = state;
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
		for (int i = 0; i < node->children.size(); i++)
			accept(node->children[i].get());
	}

	void DefaultAstHandler::accept(AstNode *node)
	{
		if (node == nullptr)
		{
			return;
		}

		switch (node->nodeType)
		{
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
			state.errors.push_back({ErrorType::INTERNAL_ERROR, node->location, state.filepath});
			break;
		}
	}

	void DefaultAstHandler::accept(ImportAst *node)
	{
		if (level != -1)
			state.errors.push_back({ ErrorType::IMPORT_OUTSIDE_GLOBAL, node->location, state.filepath });

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
					state.errors.push_back({ ErrorType::MODULE_NOT_FOUND, node->location, state.filepath, node->value });
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

	void DefaultAstHandler::accept(StatementAst *node)
	{
	}

	void DefaultAstHandler::accept(BlockAst *node)
	{
		for (int i = 0; i < node->children.size(); i++)
			accept(node->children[i].get());
	}

	void DefaultAstHandler::accept(ExpressionAst *node)
	{
		accept(node->value.get());

		if (node->shouldClearStack)
			builder.addCommand<OpClear>(); // clear expression stack
	}

	void DefaultAstHandler::accept(BinaryOperationAst *node)
	{
		Operator op = node->op;
		auto &left = node->left;
		auto &right = node->right;

		accept(left.get());
		accept(right.get());

		switch (op)
		{
		case Operator::OP_POWER:
			builder.addCommand<OpBinaryPow>();
			break;
		case Operator::OP_MULTIPLY:
			builder.addCommand<OpBinaryMul>();
			break;
		case Operator::OP_INT_DIVIDE:
		case Operator::OP_DIVIDE:
			builder.addCommand<OpBinaryDiv>();
			break;
		case Operator::OP_MODULUS:
			builder.addCommand<OpBinaryMod>();
			break;
		case Operator::OP_ADD:
			builder.addCommand<OpBinaryAdd>();
			break;
		case Operator::OP_SUBTRACT:
			builder.addCommand<OpBinarySub>();
			break;
			/*case Operator::OP_BINARY_XOR:
			builder.addCommand<OpBinaryBitXor>();
			break;
			case Operator::OP_BINARY_AND:
			builder.addCommand<OpBinaryBitAnd>();
			break;
			case Operator::OP_BINARY_OR:
			builder.addCommand<OpBinaryBitOr>();
			break;*/
		case Operator::OP_AND:
			builder.addCommand<OpBinaryAnd>();
			break;
		case Operator::OP_OR:
			builder.addCommand<OpBinaryOr>();
			break;
		case Operator::OP_EQUALS:
			builder.addCommand<OpBinaryEql>();
			break;
		case Operator::OP_NOT_EQUAL:
			builder.addCommand<OpBinaryNotEql>();
			break;
			/*case Operator::OP_OUTPUT:
			builder.addCommand<OpBinaryOut>();
			break;
			case Operator::OP_INPUT:
			builder.addCommand<OpBinaryIn>();
			break;*/
		case Operator::OP_LESS:
			builder.addCommand<OpBinaryLT>();
			break;
		case Operator::OP_GREATER:
			builder.addCommand<OpBinaryGT>();
			break;
		case Operator::OP_GREATER_OR_EQUAL:
			builder.addCommand<OpBinaryGTE>();
			break;
		case Operator::OP_LESS_OR_EQUAL:
			builder.addCommand<OpBinaryLTE>();
			break;
		case Operator::OP_ASSIGN:
			builder.addCommand<OpBinaryAssign>();
			break;
		case Operator::OP_ADD_ASSIGN:
			builder.addCommand<OpBinaryAddAssign>();
			break;
		case Operator::OP_SUBTRACT_ASSIGN:
			builder.addCommand<OpBinarySubAssign>();
			break;
		case Operator::OP_MULTIPLY_ASSIGN:
			builder.addCommand<OpBinaryMulAssign>();
			break;
		case Operator::OP_DIVIDE_ASSIGN:
			builder.addCommand<OpBinaryDivAssign>();
			break;
			/*case Operator::OP_MODULUS_ASSIGN:
			builder.addCommand<OpBinaryModAssign>();
			break;
			case Operator::OP_BINARY_AND_ASSIGN:
			builder.addCommand<OpBinaryBitAndAssign>();
			break;
			case Operator::OP_BINARY_XOR_ASSIGN:
			builder.addCommand<OpBinaryBitXorAssign>();
			break;
			case Operator::OP_BINARY_OR_ASSIGN:
			builder.addCommand<OpBinaryBitOrAssign>();
			break;*/
		default:
			state.errors.push_back(Error(ErrorType::ILLEGAL_OPERATOR, node->location, state.filepath, getOperatorStr(op)));
			break;
		}
	}

	void DefaultAstHandler::accept(UnaryOperationAst *node)
	{
		Operator op = node->op;
		auto &val = node->value;

		accept(val.get());

		switch (op)
		{
		case Operator::OP_NOT:
			builder.addCommand<OpUnaryNot>();
			break;
		case Operator::OP_ADD:
			builder.addCommand<OpUnaryPos>();
			break;
		case Operator::OP_SUBTRACT:
			builder.addCommand<OpUnaryMinus>();
			break;
		default:
			state.errors.push_back(Error(ErrorType::ILLEGAL_OPERATOR, node->location, state.filepath, getOperatorStr(op)));
			break;
		}
	}

	void DefaultAstHandler::accept(MemberAccessAst *node)
	{
		AstNode *current = node;
		while (current != nullptr && current->nodeType == AstNodeType::AST_MEMBER_ACCESS)
		{
			auto *member = dynamic_cast<MemberAccessAst*>(current);
			current = member->next.get();
		}

		if (current != nullptr)
		{
			switch (current->nodeType)
			{
			case AstNodeType::AST_VARIABLE:
			case AstNodeType::AST_FUNCTION_CALL:
			{
				accept(current);
				return;
			}
			default:
				state.errors.push_back({ ErrorType::EXPECTED_IDENTIFIER, current->location, state.filepath });
				break;
			}
		}
		else
			state.errors.push_back({ ErrorType::EXPECTED_IDENTIFIER, node->location, state.filepath });
	}

	void DefaultAstHandler::accept(VariableDeclarationAst *node)
	{
		Level &currentLevel = levels[level];
		if (std::find(currentLevel.variableNames.begin(), currentLevel.variableNames.end(), node->name)
			!= currentLevel.variableNames.end())
		{
			state.errors.push_back(Error(ErrorType::VARIABLE_REDECLARED, node->location, state.filepath, node->name));
		}
		else
		{
			currentLevel.variableNames.push_back(node->name);
			builder.addCommand<VarCreate>(VarType::VAR_TYPE_ANY, node->name);
		}
	}

	void DefaultAstHandler::accept(VariableAst *node)
	{
		if (!varInScope(node->name))
			state.errors.push_back(Error(ErrorType::VARIABLE_UNDECLARED, node->location, state.filepath, node->name));
		else
			builder.addCommand<LoadVariable>(node->name);
	}

	void DefaultAstHandler::accept(IntegerAst *node)
	{
		builder.addCommand<LoadInteger>(node->value);
	}

	void DefaultAstHandler::accept(FloatAst *node)
	{
		builder.addCommand<LoadFloat>(node->value);
	}

	void DefaultAstHandler::accept(StringAst *node)
	{
		builder.addCommand<LoadString>(node->value);
	}

	void DefaultAstHandler::accept(TrueAst *node)
	{
		builder.addCommand<LoadInteger>(1);
	}

	void DefaultAstHandler::accept(FalseAst *node)
	{
		builder.addCommand<LoadInteger>(0);
	}

	void DefaultAstHandler::accept(NullAst *node)
	{
		builder.addCommand<LoadNull>();
	}

	void DefaultAstHandler::accept(FunctionDefinitionAst *node)
	{
		levels[level].functionDeclarations.push_back(node);

		functionDefBlockIds[node] = blockIdNum;
		builder.addCommand<CreateBlock>(BlockType::FUNCTION_BLOCK,
			blockIdNum++,
			level);

		auto *fnBody = dynamic_cast<BlockAst*>(node->block.get());

		// add return statement
		if (fnBody->children.size() == 0 || 
			fnBody->children.back()->nodeType != AstNodeType::AST_RETURN_STATEMENT)
		{
			auto returnValue = std::make_unique<NullAst>(node->block->location);
			auto returnStatement = std::make_unique<ReturnStatementAst>(node->block->location, std::move(returnValue));
			fnBody->addChild(std::move(returnStatement));
		}

		increaseBlock(BlockType::FUNCTION_BLOCK);

		for (int i = node->arguments.size() - 1; i >= 0; i--)
		{
			const std::string &str = node->arguments[i];
			levels[level].variableNames.push_back(str);

			builder.addCommand<VarCreate>(VarType::VAR_TYPE_ANY, str);
			builder.addCommand<StackPopObject>(str, StackType::STACK_FUNCTION_PARAM);
		}

		accept(fnBody);
		decreaseBlock();
	}

	void DefaultAstHandler::accept(FunctionCallAst *node)
	{
		FunctionDefinitionAst *definition = nullptr;
		ReturnMessage msg;

		msg = fnInScope(node->name, node->arguments.size(), definition);

		if (msg == FN_NOT_FOUND)
			state.errors.push_back({ ErrorType::FUNCTION_NOT_FOUND, node->location, state.filepath, node->name });
		else if (msg == FN_TOO_MANY_ARGS)
			state.errors.push_back({ ErrorType::TOO_MANY_ARGS, node->location, state.filepath, node->name });
		else if (msg == FN_TOO_FEW_ARGS)
			state.errors.push_back({ ErrorType::TOO_FEW_ARGS, node->location, state.filepath, node->name });
		else if (msg == FN_FOUND)
		{
			for (int i = node->arguments.size() - 1; i >= 0; i--)
			{
				builder.addCommand<IncreaseReadLevel>();
				increaseBlock(BlockType::UNDEFINED_BLOCK);

				accept(node->arguments[i].get());
				builder.addCommand<OpPush>(StackType::STACK_FUNCTION_PARAM);

				decreaseBlock();
			}

			// Call the function. The variable name is passed so that it can be set to the result
			if (!definition->isNative)
				builder.addCommand<CallFunction>(functionDefBlockIds[definition]);
			else
				builder.addCommand<CallNativeFunction>(definition->name, functionDefBlockIds[definition], definition->arguments.size());
		}
	}

	void DefaultAstHandler::accept(IfStatementAst *node)
	{
		accept(node->cond_expr.get());
		builder.addCommand<IfStatement>();

		builder.addCommand<CreateBlock>(BlockType::IF_STATEMENT_BLOCK,
			blockIdNum++,
			level);

		increaseBlock(BlockType::IF_STATEMENT_BLOCK);
		accept(node->block.get());
		decreaseBlock();

		if (node->elseStatement != nullptr)
		{
			builder.addCommand<ElseStatement>();
			increaseBlock(BlockType::ELSE_STATEMENT_BLOCK);
			accept(node->elseStatement.get());
			decreaseBlock();
		}
	}

	void DefaultAstHandler::accept(ReturnStatementAst *node)
	{
		accept(node->value.get());
		builder.addCommand<OpPush>(StackType::STACK_FUNCTION_CALLBACK);

		int startLevel = level;
		Level *tmpFrame = &levels[startLevel];

		while (startLevel >= -1 &&
			tmpFrame->type != BlockType::FUNCTION_BLOCK)
		{
			//TODO: clear vars from block
			builder.addCommand<LeaveBlock>();
			startLevel--;//tmpFrame->parentFrame;
			tmpFrame = &levels[startLevel];
		}
		//TODO: clear vars from final block
		builder.addCommand<LeaveFunction>();
	}

	void DefaultAstHandler::accept(ForLoopAst *node)
	{
	}

	bool DefaultAstHandler::varInScope(const std::string &name)
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

	ReturnMessage DefaultAstHandler::fnInScope(const std::string &name, int nArgs, FunctionDefinitionAst *&out)
	{
		int startLevel = level;
		ReturnMessage status = FN_NOT_FOUND;

		while (startLevel >= -1)
		{
			Level &currentLevel = levels.at(startLevel);

			for (int i = 0; i < currentLevel.functionDeclarations.size(); i++)
			{
				if (currentLevel.functionDeclarations[i]->name == name)
				{
					if (currentLevel.functionDeclarations[i]->arguments.size() == nArgs)
					{
						out = currentLevel.functionDeclarations[i];
						return FN_FOUND;
					}
					else if (currentLevel.functionDeclarations[i]->arguments.size() < nArgs)
						status = FN_TOO_MANY_ARGS;
					else if (currentLevel.functionDeclarations[i]->arguments.size() > nArgs)
						status = FN_TOO_FEW_ARGS;
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

		builder.addCommand<IncreaseBlockLevel>();
	}

	void DefaultAstHandler::decreaseBlock()
	{
		levels[level--] = Level();
		builder.addCommand<DecreaseBlockLevel>();
	}

	void DefaultAstHandler::defineFunction(const std::string &name, size_t numArgs)
	{
		std::vector<std::string> args;
		for (size_t i = 0; i < numArgs; i++)
		{
			std::string arg = "$" + name + "_arg" + std::to_string(i);
			args.push_back(arg);
		}

		std::unique_ptr<FunctionDefinitionAst> definition
			= std::make_unique<FunctionDefinitionAst>(SourceLocation(-1, -1, ""),
				name,
				args,
				nullptr,
				true);
		nativeFunctions.push_back(std::move(definition));
		levels[level].functionDeclarations.push_back(nativeFunctions.back().get());

		functionDefBlockIds[nativeFunctions.back().get()] = blockIdNum;
		builder.addCommand<CreateBlock>(BlockType::FUNCTION_BLOCK,
			blockIdNum++,
			level);
	}
}
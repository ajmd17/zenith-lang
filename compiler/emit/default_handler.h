#ifndef __ZENITH_COMPILER_DEFAULT_HANDLER_H__
#define __ZENITH_COMPILER_DEFAULT_HANDLER_H__

#include <vector>
#include <string>
#include <map>
#include <utility>

#include "../ast.h"
#include "../state.h"

namespace zenith
{
	namespace compiler
	{
		struct Level
		{
			std::vector<FunctionDefinitionAst*> functionDeclarations;
			std::vector<std::string> variableNames;

			BlockType type;
		};

		enum ReturnMessage
		{
			FN_TOO_MANY_ARGS,
			FN_TOO_FEW_ARGS,
			FN_NOT_FOUND,
			FN_FOUND,

			VAR_NOT_FOUND,
			VAR_FOUND
		};

		class DefaultAstHandler : public AstHandler
		{
		private:
			BytecodeCommandList commandList;

			std::vector<std::shared_ptr<FunctionDefinitionAst>> nativeFunctions;

			std::map<std::string, std::unique_ptr<ModuleAst>> externalModules;

			int blockIdNum = 0;
			std::map<FunctionDefinitionAst*, int> functionDefBlockIds;

			int level = -1; // todo: map module name to levels
			std::map<int, Level> levels;

			bool varInScope(const std::string &name);
			ReturnMessage fnInScope(const std::string &name, int nArgs, FunctionDefinitionAst *&out);

			ParserState state;

			void increaseBlock(BlockType type);
			void decreaseBlock();

			BytecodeCommandList &getCommand() { return commandList; }

			template <typename T, typename ... Args>
			typename std::enable_if<std::is_base_of<BytecodeCommand, T>::value, void>::type
				addCommand(Args... args)
			{
				auto tPtr = std::make_shared<T>(args...);
				commandList.push_back(tPtr);
			}

		public:
			DefaultAstHandler(ParserState &state);
			~DefaultAstHandler();

			void accept(ModuleAst *node);

			ParserState &getState() { return state; }

			void defineFunction(const std::string &name, size_t numArgs);

		protected:
			void accept(AstNode *node);
			void accept(ImportsAst *node);
			void accept(ImportAst *node);
			void accept(StatementAst *node);
			void accept(BlockAst *node);
			void accept(ExpressionAst *node);
			void accept(BinaryOperationAst *node);
			void accept(UnaryOperationAst *node);
			void accept(MemberAccessAst *node);
			void accept(VariableDeclarationAst *node);
			void accept(VariableAst *node);
			void accept(IntegerAst *node);
			void accept(FloatAst *node);
			void accept(StringAst *node);
			void accept(TrueAst *node);
			void accept(FalseAst *node);
			void accept(NullAst *node);
			void accept(FunctionDefinitionAst *node);
			void accept(FunctionCallAst *node);
			void accept(IfStatementAst *node);
			void accept(ReturnStatementAst *node);
			void accept(ForLoopAst *node);
		};
	}
}

#endif
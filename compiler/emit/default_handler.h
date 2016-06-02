#ifndef __ZENITH_COMPILER_DEFAULT_HANDLER_H__
#define __ZENITH_COMPILER_DEFAULT_HANDLER_H__

#include <vector>
#include <string>
#include <map>
#include <utility>

#include "bytecode.h"
#include "../ast.h"
#include "../state.h"

namespace zenith
{
	namespace compiler
	{
		struct VariableInfo
		{
			bool isClass = false;
			ClassAst *classType = nullptr;

			VariableInfo() {}
			VariableInfo(bool isClass, ClassAst *classType)
			{
				this->isClass = isClass;
				this->classType = classType;
			}
		};

		struct Level
		{
			// maps the 'mangled' function names to their definitions
			std::vector<
				std::pair<
					std::string, 
					FunctionDefinitionAst*
				>
			> functionDeclarations;
			// holds all variable names
			std::map<
					std::string,
					VariableInfo
			> variableNames;
			// is it a function, if statement, loop, etc.
			BlockType type;
		};

		enum ReturnMessage
		{
			FN_NOT_FOUND,
			FN_TOO_MANY_ARGS,
			FN_TOO_FEW_ARGS,
			FN_FOUND,

			VAR_NOT_FOUND,
			VAR_FOUND
		};

		class DefaultAstHandler : public AstHandler
		{
		private:
			BytecodeCommandList commandList;

			std::vector<
				std::shared_ptr<
				FunctionDefinitionAst
				>
			> nativeFunctions;

			std::map<
				std::string, 
				std::unique_ptr<
					ModuleAst
				>
			> externalModules;
			ModuleAst *mainModule;

			std::map<
				std::string, 
				ClassAst*
			> classTypes;

			std::map<
				std::string,
				unsigned long
			> classInstanceId;

			std::pair<
				std::string, 
				ClassAst*
			> self;

			static const std::string SELF_DEFAULT, SELF_GLOBAL;
			static const int LEVEL_GLOBAL;

			int blockIdNum = 0;
			std::map<FunctionDefinitionAst*, int> functionDefBlockIds;

			int level = -1;
			std::map<int, Level> levels;

			bool isModule(const std::string &name);

			bool varInScope(const std::string &name);
			bool varInScope(const std::string &name, 
				VariableInfo &outInfo);
			int getVarLevel(const std::string &name);
			ReturnMessage fnInScope(const std::string &name, 
				int nArgs, 
				FunctionDefinitionAst *&out);

			ParserState state;

			void increaseBlock(BlockType type);
			void decreaseBlock();

			std::string makeIdentifier(AstNode *moduleAst, 
				std::pair<std::string, ClassAst*> memberOf,
				const std::string &original,
				int numArguments = 0);
			std::string unmangleIdentifier(const std::string &mangled);
			bool isIdentifier(const std::string &name, 
				bool thisScopeOnly=false);

			AstNode *loopMemberAccess(MemberAccessAst *node);

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
			BytecodeCommandList &getCommands() { return commandList; }

			void defineFunction(const std::string &name, 
				const std::string &moduleName, 
				size_t numArgs);

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
			void accept(SelfAst *node);
			void accept(NewAst *node);
			void accept(FunctionDefinitionAst *node);
			void accept(FunctionCallAst *node);
			void accept(ClassAst *node);
			void accept(IfStatementAst *node);
			void accept(ReturnStatementAst *node);
			void accept(ForLoopAst *node);
		};
	}
}

#endif
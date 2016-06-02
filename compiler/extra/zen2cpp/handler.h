#ifndef __ZEN2CPP_HANDLER_H__
#define __ZEN2CPP_HANDLER_H__

#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <utility>

#include "../../ast.h"
#include "../../state.h"
#include "../../../enums.h"

namespace zenith {
	namespace compiler {
		namespace zen2cpp {
			struct Level
			{
				// maps the 'mangled' function names to their definitions
				std::map<std::string, FunctionDefinitionAst*> functionDeclarations;
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

			class Zen2CppHandler : public AstHandler
			{
			private:
				std::stringstream cppOutput;

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

				void indent(int nSpaces = 2)
				{
					for (int i = 0; i < (level + 1); i++)
						for (int j = 0; j < nSpaces; j++)
							cppOutput << " ";
				}

				template <typename T>
				void append(T t)
				{
					cppOutput << t;
				}

			public:
				Zen2CppHandler(ParserState &state);
				~Zen2CppHandler();

				void accept(ModuleAst *node);

				ParserState &getState() { return state; }

				std::string getOutput() const { return cppOutput.str(); }

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
}

#endif
#ifndef __COMPILER2_H__
#define __COMPILER2_H__

#include <vector>
#include <string>
#include <map>
#include <utility>

#include "bytecode.h"
#include "../ast.h"
#include "../state.h"

using std::map;
using std::vector;
using std::string;

namespace zenith
{
	namespace compiler
	{
		struct ClassTypeRepr
		{

		};

		struct ObjectRepr
		{
		};

		struct ScopeRepr
		{
			map<string, ObjectRepr> objects;

			// is it a function, if statement, loop, etc.
			BlockType type;
		};

		// representation of a module
		struct ModuleRepr
		{
			map<string, ClassTypeRepr> classTypes;
			map<int, ScopeRepr> scopes;

			ModuleRepr();
		};

		struct CompilerState
		{
			BytecodeCommandList commands;

			map<string, ModuleRepr> modules;
			vector<string> moduleFilepaths;
			ModuleRepr *currentModule = nullptr;

			std::pair<string, int> selfInfo; // the name of 'self' variable

			vector<Error> errors;

			int level = -1;

			void increaseBlock(BlockType type);
			void decreaseBlock();

			bool moduleImported(const string &);

			template <typename T, typename ... Args>
			typename std::enable_if<std::is_base_of<BytecodeCommand, T>::value, void>::type
				addCommand(Args... args)
			{
				auto tPtr = std::make_shared<T>(args...);
				commands.push_back(tPtr);
			}

			void newObject(string &name);
			void newObject(string &name, ModuleRepr *module);
			void newClassType(string &name);
			void newClassType(string &name, ModuleRepr *module);

		};

		class Compiler
		{
		private:
			CompilerState state;

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

		public:
			Compiler(ParserState &parserState);
			~Compiler();

			void accept(ModuleAst *node);

			CompilerState &getState() { return state; }
		};

		class Util
		{
		public:
			static bool findVariable(CompilerState &state,
				const string &name,
				bool thisScopeOnly = false);
			static bool findVariable(CompilerState &state,
				const string &name,
				int scope,
				bool thisScopeOnly = false);
			static bool legalIdentifier(CompilerState &state,
				const string &name);
		};

	}
}

#endif
#ifndef __ZENITH_COMPILER_EMITTER_H__
#define __ZENITH_COMPILER_EMITTER_H__

#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <stdint.h>
#include <memory>

#include "bytecode.h"
#include "../state.h"
#include "../../enums.h"
#include "../ast.h"

namespace zenith
{
	namespace compiler
	{
		struct BlockCreate
		{
			int32_t blockId;
			int32_t blockType;
			int32_t parentId;
			uint64_t blockPos;
		};

		struct ExternalFunctionDefine
		{
			std::string name;
			std::string moduleName;
			size_t nArgs;
		};

		class Emitter
		{
		private:
			std::ofstream filestream;
			std::vector<BlockCreate> block_creates;
			int appendOffset; /* the offset will be incremented every time createBlock() is called,
								because in the end we will have to append to file. */
			bool hideVariableNames;
			bool closed;
			bool append;
			bool bigEndian;
			bool writeLabelsToBeginning;

			std::streampos lastPosition;

			ModuleAst *unit;
			ParserState state;

			std::vector<ExternalFunctionDefine> externalFunctions;

		public:
			Emitter(ModuleAst *unit, ParserState &state);
			~Emitter();

			std::streampos getLastPosition() { return lastPosition; }

			bool emit(const std::string &filepath);

			void defineFunction(ExternalFunctionDefine func) { externalFunctions.push_back(func); }

		private:
			void close();

			void increaseBlockLevel();
			void decreaseBlockLevel();
			void increaseReadLevel();
			void decreaseReadLevel();
			void leaveBlock();
			void stackPopObject(std::string &varName, int whichStack);
			void createBlock(unsigned int blockId, BlockType blockType, unsigned int parentId);
			void goToBlock(unsigned int blockId);
			void goToIfTrue(unsigned int blockId);
			void goToIfFalse(unsigned int blockId);
			void createClass(unsigned int blockId);
			void addMember(const std::string &name);
			void loadMember(const std::string &name);
			void invoke();
			void callNativeFunction(unsigned int blockId, const std::string &name, unsigned int numArgs);
			void createFunction(const std::string &funName);
			void createNativeClassInstance(const std::string &className);
			void leaveFunction();
			void pushFunctionChain();
			void popFunctionChain();
			void ifStatement();
			void elseStatement();
			void leaveIfStatement();
			void leaveElseStatement();
			void createVariable(VarType varType, const std::string &varName);
			void varAddProperty(const std::string &varName, const std::string &propertyName);
			void varPushProperty(const std::string &varName, const std::string &propertyName);
			void clearVariable(const std::string &varName);
			void deleteVariable(const std::string &varName);
			void loopBreak(int levelsToSkip);
			void loopContinue(int levelsToSkip);
			void loadVariable(const std::string &varName);
			void loadInteger(long value);
			void loadFloat(double value);
			void loadString(const std::string &value);
			void loadNull();
			void opPush(int whichStack);
			void op(Instruction operation);
		};
	}
}

#endif
#include "emitter.h"

#include <cstdint>
#include <memory>
#include <algorithm>

#include "../ast.h"
#include "default_handler.h"

namespace zenith
{
	namespace compiler
	{
		Emitter::Emitter(ModuleAst *unit, ParserState &state)
		{
			this->unit = unit;
			this->state = state;

			this->block_creates.clear();
			this->closed = false;
			this->appendOffset = 0;
		}

		Emitter::~Emitter()
		{
			if (!closed)
				this->close();
		}

		void Emitter::close()
		{
			if (!closed)
			{
				this->closed = true;
				this->filestream.close();
			}
		}

		bool Emitter::emit(const std::string &filepath)
		{
			DefaultAstHandler handler(state);

			for (ExternalFunctionDefine func : externalFunctions)
			{
				handler.defineFunction(func.name, func.moduleName, func.nArgs);
			}

			handler.accept(unit);
			state = handler.getState();

			if (state.errors.size() == 0)
			{
				BytecodeCommandList commandList = handler.getCommands();

				this->filestream.open(filepath, std::ofstream::binary);
				if (!filestream.is_open())
				{
					std::cout << "Could not open file: " << filepath << "\n";
					throw std::runtime_error("Could not open file");
				}

				if (writeLabelsToBeginning)
				{
					// add CreateBlock commands at beginning of file
					for (unsigned long i = 0; i < commandList.size(); i++)
					{
						if (commandList[i]->command == Instruction::CMD_CREATE_BLOCK)
						{
							auto cmd = std::static_pointer_cast<CreateBlock>(commandList[i]);
							this->createBlock(cmd->blockId, cmd->blockType, cmd->parentId);
						}
					}
				}

				for (unsigned long i = 0; i < commandList.size(); i++)
				{
					switch (commandList[i]->command)
					{
					case Instruction::CMD_ADD_PROPERTY:
					{
						auto cmd = std::static_pointer_cast<VarAddProperty>(commandList[i]);
						this->varAddProperty(cmd->varName, cmd->propertyName);

						break;
					}
					case Instruction::CMD_CALL_FUNCTION:
					{
						auto cmd = std::static_pointer_cast<CallFunction>(commandList[i]);
						this->callFunction(/*cmd->blockId*/cmd->functionName);

						break;
					}
					case Instruction::CMD_INVOKE_METHOD:
					{
						auto cmd = std::static_pointer_cast<InvokeMethod>(commandList[i]);
						this->invokeMethod(cmd->functionName);

						break;
					}
					case Instruction::CMD_CALL_NATIVE_FUNCTION:
					{
						auto cmd = std::static_pointer_cast<CallNativeFunction>(commandList[i]);
						this->callNativeFunction(cmd->blockId, cmd->fnName, cmd->numArgs);

						break;
					}
					case Instruction::CMD_CREATE_NATIVE_CLASS_INSTANCE:
					{
						auto cmd = std::static_pointer_cast<CreateNativeClassInstance>(commandList[i]);
						this->createNativeClassInstance(cmd->className);

						break;
					}
					case Instruction::CMD_GO_TO_BLOCK:
					{
						auto cmd = std::static_pointer_cast<GoToBlock>(commandList[i]);
						this->goToBlock(cmd->blockId);

						break;
					}
					case Instruction::CMD_GO_TO_IF_TRUE:
					{
						auto cmd = std::static_pointer_cast<GoToIfTrue>(commandList[i]);
						this->goToIfTrue(cmd->blockId);

						break;
					}
					case Instruction::CMD_GO_TO_IF_FALSE:
					{
						auto cmd = std::static_pointer_cast<GoToIfFalse>(commandList[i]);
						this->goToIfFalse(cmd->blockId);

						break;
					}
					case Instruction::CMD_CREATE_FUNCTION:
					{
						auto cmd = std::static_pointer_cast<CreateFunction>(commandList[i]);
						this->createFunction(cmd->functionName);

						break;
					}
					case Instruction::CMD_CREATE_VAR:
					{
						auto cmd = std::static_pointer_cast<VarCreate>(commandList[i]);
						this->createVariable(cmd->varType, cmd->varName);

						break;
					}
					case Instruction::CMD_CREATE_BLOCK:
					{
						if (!writeLabelsToBeginning)
						{
							auto cmd = std::static_pointer_cast<CreateBlock>(commandList[i]);
							this->createBlock(cmd->blockId, cmd->blockType, cmd->parentId);
						}
						break;
					}
					case Instruction::CMD_DEC_BLOCK_LEVEL:
					{
						this->decreaseBlockLevel();

						break;
					}
					case Instruction::CMD_DEC_READ_LEVEL:
					{
						this->decreaseReadLevel();

						break;
					}
					case Instruction::CMD_CLEAR_VAR:
					{
						auto cmd = std::static_pointer_cast<DeleteObject>(commandList[i]);
						this->clearVariable(cmd->varName);

						break;
					}
					case Instruction::CMD_ELSE_STATEMENT:
					{
						this->elseStatement();

						break;
					}
					case Instruction::CMD_IF_STATEMENT:
					{
						this->ifStatement();

						break;
					}
					case Instruction::CMD_INC_BLOCK_LEVEL:
					{
						this->increaseBlockLevel();

						break;
					}
					case Instruction::CMD_INC_READ_LEVEL:
					{
						this->increaseReadLevel();

						break;
					}
					case Instruction::CMD_LEAVE_BLOCK:
					{
						this->leaveBlock();

						break;
					}
					case Instruction::CMD_LEAVE_ELSE_STATEMENT:
					{
						this->leaveElseStatement();

						break;
					}
					case Instruction::CMD_LEAVE_FUNCTION:
					{
						this->leaveFunction();

						break;
					}
					case Instruction::CMD_LEAVE_IF_STATEMENT:
					{
						this->leaveIfStatement();

						break;
					}
					case Instruction::CMD_POP_FUNCTION_CHAIN:
					{
						this->popFunctionChain();

						break;
					}
					case Instruction::CMD_PUSH_FUNCTION_CHAIN:
					{
						this->pushFunctionChain();

						break;
					}
					case Instruction::CMD_PUSH_PROPERTY:
					{
						auto cmd = std::static_pointer_cast<VarPushProperty>(commandList[i]);
						this->varPushProperty(cmd->varName, cmd->propertyName);

						break;
					}
					case Instruction::CMD_DELETE_VAR:
					{
						auto cmd = std::static_pointer_cast<RemoveReference>(commandList[i]);
						this->deleteVariable(cmd->varName);

						break;
					}
					case Instruction::CMD_STACK_POP_OBJECT:
					{
						auto cmd = std::static_pointer_cast<StackPopObject>(commandList[i]);
						this->stackPopObject(cmd->varName, cmd->whichStack);

						break;
					}
					case Instruction::CMD_LOOP_BREAK:
					{
						auto cmd = std::static_pointer_cast<LoopBreak>(commandList[i]);
						this->loopBreak(cmd->levelsToSkip);

						break;
					}
					case Instruction::CMD_LOOP_CONTINUE:
					{
						auto cmd = std::static_pointer_cast<LoopContinue>(commandList[i]);
						this->loopContinue(cmd->levelsToSkip);

						break;
					}
					case Instruction::CMD_LOAD_INTEGER:
					{
						auto cmd = std::static_pointer_cast<LoadInteger>(commandList[i]);
						this->loadInteger(cmd->val);

						break;
					}
					case Instruction::CMD_LOAD_FLOAT:
					{
						auto cmd = std::static_pointer_cast<LoadFloat>(commandList[i]);
						this->loadFloat(cmd->val);

						break;
					}
					case Instruction::CMD_LOAD_STRING:
					{
						auto cmd = std::static_pointer_cast<LoadString>(commandList[i]);
						this->loadString(cmd->val);

						break;
					}
					case Instruction::CMD_LOAD_NULL:
						this->loadNull();
						break;
					case Instruction::CMD_LOAD_VARIABLE:
					{
						auto cmd = std::static_pointer_cast<LoadVariable>(commandList[i]);
						this->loadVariable(cmd->val);

						break;
					}
					case Instruction::CMD_OP_PUSH:
					{
						auto cmd = std::static_pointer_cast<OpPush>(commandList[i]);
						this->opPush(cmd->whichStack);

						break;
					}
					case Instruction::CMD_OP_CLEAR:
					case Instruction::CMD_OP_UNARY_NEG:
					case Instruction::CMD_OP_UNARY_POS:
					case Instruction::CMD_OP_UNARY_NOT:
					case Instruction::CMD_OP_ADD:
					case Instruction::CMD_OP_SUB:
					case Instruction::CMD_OP_MUL:
					case Instruction::CMD_OP_DIV:
					case Instruction::CMD_OP_MOD:
					case Instruction::CMD_OP_POW:
					case Instruction::CMD_OP_AND:
					case Instruction::CMD_OP_OR:
					case Instruction::CMD_OP_EQL:
					case Instruction::CMD_OP_NEQL:
					case Instruction::CMD_OP_LT:
					case Instruction::CMD_OP_GT:
					case Instruction::CMD_OP_LTE:
					case Instruction::CMD_OP_GTE:
					case Instruction::CMD_OP_ASSIGN:
					case Instruction::CMD_OP_ADD_ASSIGN:
					case Instruction::CMD_OP_SUB_ASSIGN:
					case Instruction::CMD_OP_MUL_ASSIGN:
					case Instruction::CMD_OP_DIV_ASSIGN:
						this->op(commandList[i]->command);
						break;
					default:
						break;
					}
				}

				this->close();
				return true;
			}
			else
			{
				// map the filepath to vector of errors
				std::map<
					std::string, 
					std::vector<
						Error
					>
				> errorMap;

				for (auto &&it : state.errors)
				{
					if (errorMap.find(it.location.file) == errorMap.end())
						errorMap.insert({ it.location.file, std::vector<Error>() });

					errorMap[it.location.file].push_back(it);
				}

				for (auto it = errorMap.rbegin(); it != errorMap.rend(); ++it)
				{
					std::sort(it->second.begin(), it->second.end());

					std::cout << "Errors in file: " << it->first << "\n";

					for (auto &&error : it->second)
						error.display();
				}
			}

			this->close();
			return false;
		}

		void Emitter::increaseBlockLevel()
		{
			int32_t type = Instruction::CMD_INC_BLOCK_LEVEL;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::decreaseBlockLevel()
		{
			int32_t type = Instruction::CMD_DEC_BLOCK_LEVEL;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::increaseReadLevel()
		{
			int32_t type = Instruction::CMD_INC_READ_LEVEL;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::decreaseReadLevel()
		{
			int32_t type = Instruction::CMD_DEC_READ_LEVEL;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::leaveBlock()
		{
			int32_t type = Instruction::CMD_LEAVE_BLOCK;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::stackPopObject(std::string &varName, int whichStack)
		{
			int32_t type = Instruction::CMD_STACK_POP_OBJECT;
			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t wstack = (int32_t)whichStack;
			this->filestream.write((char*)&wstack, sizeof(int32_t));

			int32_t varNameLen = varName.length() + 1;

			this->filestream.write((char*)&varNameLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varNameLen);
		}

		void Emitter::createClass(unsigned int blockId)
		{
		}

		void Emitter::createBlock(unsigned int blockId, BlockType blockType, unsigned int parentId)
		{
			appendOffset = 0;

			appendOffset += sizeof(int32_t); // cmd type
			appendOffset += sizeof(int32_t); // blockid
			appendOffset += sizeof(int32_t); // block type
			appendOffset += sizeof(int32_t); // parent id
			appendOffset += sizeof(uint64_t); // block pos

			BlockCreate blockCreate;
			blockCreate.blockId = blockId;
			blockCreate.blockType = (int32_t)blockType;
			blockCreate.parentId = parentId;
			blockCreate.blockPos = ((uint64_t)filestream.tellp());

			uint64_t bPos = blockCreate.blockPos;
			bPos += appendOffset;

			int32_t type = Instruction::CMD_CREATE_BLOCK;

			filestream.write((char*)&type, sizeof(int32_t));
			filestream.write((char*)&blockCreate.blockId, sizeof(int32_t));
			filestream.write((char*)&blockCreate.blockType, sizeof(int32_t));
			filestream.write((char*)&blockCreate.parentId, sizeof(int32_t));
			filestream.write((char*)&bPos, sizeof(uint64_t));
		}

		void Emitter::goToBlock(unsigned int blockId)
		{
			int32_t type = Instruction::CMD_GO_TO_BLOCK;
			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&blockId, sizeof(int32_t));
		}

		void Emitter::goToIfTrue(unsigned int blockId)
		{
			int32_t type = Instruction::CMD_GO_TO_IF_TRUE;
			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&blockId, sizeof(int32_t));
		}

		void Emitter::goToIfFalse(unsigned int blockId)
		{
			int32_t type = Instruction::CMD_GO_TO_IF_FALSE;
			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&blockId, sizeof(int32_t));
		}

		void Emitter::callNativeFunction(unsigned int blockId, const std::string &name, unsigned int numArgs)
		{
			int32_t type = Instruction::CMD_CALL_NATIVE_FUNCTION;
			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&blockId, sizeof(int32_t));
			this->filestream.write((char*)&numArgs, sizeof(int32_t));

			int32_t varNameLen = name.length() + 1;

			this->filestream.write((char*)&varNameLen, sizeof(int32_t));
			this->filestream.write(name.c_str(), varNameLen);
		}

		void Emitter::createFunction(const std::string &funName)
		{
			int32_t type = Instruction::CMD_CREATE_FUNCTION;

			int32_t funNameLen = funName.length() + 1;

			appendOffset = 0;

			appendOffset += sizeof(int32_t); // cmd type
			appendOffset += sizeof(int32_t); // name length integer
			appendOffset += funNameLen;
			appendOffset += sizeof(uint64_t); // block pos

			uint64_t bPos = ((uint64_t)filestream.tellp());
			bPos += appendOffset;

			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&funNameLen, sizeof(int32_t));
			this->filestream.write(funName.c_str(), funNameLen);

			this->filestream.write((char*)&bPos, sizeof(uint64_t));
		}

		void Emitter::createNativeClassInstance(const std::string &className)
		{
			int32_t type = Instruction::CMD_CREATE_NATIVE_CLASS_INSTANCE;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t classNameLen = className.length() + 1;
			this->filestream.write((char*)&classNameLen, sizeof(int32_t));

			this->filestream.write(className.c_str(), classNameLen);
		}

		void Emitter::callFunction(const std::string &name/*unsigned int blockId*/)
		{
			int32_t type = Instruction::CMD_CALL_FUNCTION;

			this->filestream.write((char*)&type, sizeof(int32_t));
			//this->filestream.write((char*)&blockId, sizeof(int32_t));

			int32_t nameLen = name.length() + 1;
			this->filestream.write((char*)&nameLen, sizeof(int32_t));

			this->filestream.write(name.c_str(), nameLen);
		}

		void Emitter::invokeMethod(const std::string &name)
		{
			int32_t type = Instruction::CMD_INVOKE_METHOD;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t nameLen = name.length() + 1;
			this->filestream.write((char*)&nameLen, sizeof(int32_t));

			this->filestream.write(name.c_str(), nameLen);
		}

		void Emitter::leaveFunction()
		{
			int32_t type = Instruction::CMD_LEAVE_FUNCTION;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::pushFunctionChain()
		{
			int32_t type = Instruction::CMD_PUSH_FUNCTION_CHAIN;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::popFunctionChain()
		{
			int32_t type = Instruction::CMD_POP_FUNCTION_CHAIN;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::ifStatement()
		{
			int32_t type = Instruction::CMD_IF_STATEMENT;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::elseStatement()
		{
			int32_t type = Instruction::CMD_ELSE_STATEMENT;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::leaveIfStatement()
		{
			int32_t type = Instruction::CMD_LEAVE_IF_STATEMENT;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::leaveElseStatement()
		{
			int32_t type = Instruction::CMD_LEAVE_ELSE_STATEMENT;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::varAddProperty(const std::string &varName, const std::string &propertyName)
		{
			int32_t type = Instruction::CMD_ADD_PROPERTY;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t varLen = varName.length() + 1;
			int32_t propLen = propertyName.length() + 1;

			this->filestream.write((char*)&varLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);

			this->filestream.write((char*)&propLen, sizeof(int32_t));
			this->filestream.write(propertyName.c_str(), propLen);
		}

		void Emitter::varPushProperty(const std::string &varName, const std::string &propertyName)
		{
			int32_t type = Instruction::CMD_PUSH_PROPERTY;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t varLen = varName.length() + 1;
			int32_t propLen = propertyName.length() + 1;

			this->filestream.write((char*)&varLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);

			this->filestream.write((char*)&propLen, sizeof(int32_t));
			this->filestream.write(propertyName.c_str(), propLen);
		}

		void Emitter::createVariable(VarType varType, const std::string &varName)
		{
			int32_t type = Instruction::CMD_CREATE_VAR;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t vType = (int32_t)varType;
			int32_t varLen = varName.length() + 1;

			this->filestream.write(reinterpret_cast<char*>(&vType), sizeof(int32_t));
			this->filestream.write(reinterpret_cast<char*>(&varLen), sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);
		}

		void Emitter::clearVariable(const std::string &varName)
		{
			int32_t type = Instruction::CMD_CLEAR_VAR;

			int32_t varLen = varName.length() + 1;

			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&varLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);
		}

		void Emitter::deleteVariable(const std::string &varName)
		{
			int32_t type = Instruction::CMD_DELETE_VAR;

			int32_t varLen = varName.length() + 1;

			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&varLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);
		}

		void Emitter::loopBreak(int levelsToSkip)
		{
			int32_t type = Instruction::CMD_LOOP_BREAK;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t lvls = (int32_t)levelsToSkip;
			this->filestream.write((char*)&lvls, sizeof(int32_t));
		}

		void Emitter::loopContinue(int levelsToSkip)
		{
			int32_t type = Instruction::CMD_LOOP_CONTINUE;

			this->filestream.write((char*)&type, sizeof(int32_t));

			int32_t lvls = (int32_t)levelsToSkip;
			this->filestream.write((char*)&lvls, sizeof(int32_t));
		}

		void Emitter::loadVariable(const std::string &varName)
		{
			int32_t type = Instruction::CMD_LOAD_VARIABLE;

			int32_t varLen = varName.length() + 1;

			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&varLen, sizeof(int32_t));
			this->filestream.write(varName.c_str(), varLen);
		}

		void Emitter::loadInteger(long value)
		{
			int32_t type = Instruction::CMD_LOAD_INTEGER;

			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&value, sizeof(long));
		}

		void Emitter::loadFloat(double value)
		{
			int32_t type = Instruction::CMD_LOAD_FLOAT;

			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&value, sizeof(double));
		}

		void Emitter::loadString(const std::string &strValue)
		{
			int32_t type = Instruction::CMD_LOAD_STRING;

			int32_t strLen = strValue.length() + 1;

			this->filestream.write((char*)&type, sizeof(int32_t));

			this->filestream.write((char*)&strLen, sizeof(int32_t));
			this->filestream.write(strValue.c_str(), strLen);
		}

		void Emitter::loadNull()
		{
			int32_t type = Instruction::CMD_LOAD_NULL;
			this->filestream.write((char*)&type, sizeof(int32_t));
		}

		void Emitter::opPush(int whichStack)
		{
			int32_t type = Instruction::CMD_OP_PUSH;

			int32_t wStack = (int32_t)whichStack;

			this->filestream.write((char*)&type, sizeof(int32_t));
			this->filestream.write((char*)&wStack, sizeof(int32_t));
		}

		void Emitter::op(Instruction operation)
		{
			int32_t type = operation;

			this->filestream.write((char*)&type, sizeof(int32_t));
		}
	}
}
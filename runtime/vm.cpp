#include "vm.h"

#include "experimental/vm_state.h"
#include "experimental/function.h"
#include "experimental/object.h"

#include "../util/logger.h"
#include "../util/timer.h"

namespace zenith
{
	using namespace util;

	namespace runtime
	{
		VM::VM(VMState *state)
		{
			this->state = state;
			this->state->vm = this;

			for (int i = 0; i < 4; i++)
				objectStacks.push_back(ObjectStack());

			blockLevel = -1;
		}

		VM::~VM()
		{
			objectStacks.clear();

			/*int startLevel = blockLevel;
			while (startLevel >= -1)
				leaveFrame(startLevel--);*/
		}

		void VM::handleInstruction(Instruction ins, Module *module)
		{
			switch (ins)
			{
			case Instruction::CMD_INC_BLOCK_LEVEL:
			{
				blockLevel++;
				module->createFrame(blockLevel);

				debug_log("Increase block level to: %d", blockLevel);
				break;
			}
			case Instruction::CMD_DEC_BLOCK_LEVEL:
			{
				if (state->readLevel == blockLevel)
				{
					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);
				}

				module->leaveFrame(blockLevel);
				blockLevel--;

				debug_log("Decrease block level to: %d", blockLevel);
				break;
			}
			case Instruction::CMD_INC_READ_LEVEL:
			{
				if (state->readLevel == blockLevel)
				{
					state->readLevel++;
					debug_log("Increase read level to: %d", state->readLevel);
				}
				break;
			}
			case Instruction::CMD_DEC_READ_LEVEL:
			{
				if (state->readLevel == blockLevel)
				{
					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);
				}
				break;
			}
			case Instruction::CMD_STACK_POP_OBJECT:
			{
				// which stack to use
				int32_t whichStack;
				if (state->readLevel == blockLevel)
					state->stream->read(&whichStack);
				else
					state->stream->skip(sizeof(int32_t));

				// load string for variable name
				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				// pop result into variable
				if (state->readLevel == blockLevel)
				{
					debug_log("Pop into value '%s' from stack %d",
						varNameStr.c_str(), whichStack);

					int startLevel = blockLevel;
					bool found = false;

					while (startLevel >= -1)
					{
						StackFrame &frame = module->getFrame(startLevel);
						if (frame.hasLocal(varNameStr))
						{
							auto &obj = frame.getLocal(varNameStr);
							obj = getObjectStack(whichStack).top();
							getObjectStack(whichStack).pop();

							debug_log("Set variable '%s' to value: '%s'",
								varNameStr.c_str(), obj->str().c_str());

							found = true;
							break;
						}

						startLevel--;
					}

					if (!found)
						throw std::runtime_error("Could not find object");
				}
				break;
			}
			case Instruction::CMD_CREATE_BLOCK:
			{
				int32_t blockId;
				state->stream->read(&blockId);

				int32_t blockType;
				state->stream->read(&blockType);

				int32_t parentId;
				state->stream->read(&parentId);

				uint64_t blockPos;
				state->stream->read(&blockPos);

				module->getSavedPositions()[blockId] = blockPos;

				debug_log("Create block: %d at position: %d", blockId, blockPos);

				break;
			}
			case Instruction::CMD_CREATE_FUNCTION:
			{
				// load string for variable name
				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				uint64_t blockPos;
				if (state->readLevel == blockLevel)
				{
					state->stream->read(&blockPos);
				}
				else
					state->stream->skip(sizeof(uint64_t));

				if (state->readLevel == blockLevel)
				{
					// create function
					if (globalFunctions.find(varNameStr) == globalFunctions.end())
					{
						auto fnPtr = std::make_shared<Function>(blockPos);
						globalFunctions[varNameStr] = fnPtr;

						debug_log("Created function: %s at position: %d", varNameStr.c_str(), blockPos);
					}
				}

				break;
			}
			case Instruction::CMD_GO_TO_BLOCK:
			{
				int32_t blockId;
				if (state->readLevel == blockLevel)
					state->stream->read(&blockId);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					auto position = module->getSavedPositions()[blockId];
					debug_log("Go to block: %d at position: %d", blockId, position);

					state->stream->seek(position);
				}

				break;
			}
			case Instruction::CMD_GO_TO_IF_TRUE:
			{
				int32_t blockId;
				if (state->readLevel == blockLevel)
					state->stream->read(&blockId);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					auto lastResult = module->getFrame(blockLevel).getLastIfResult();
					if (lastResult)
					{
						auto position = module->getSavedPositions()[blockId];
						debug_log("Go to block: %d at position: %d", blockId, position);

						state->stream->seek(position);
					}
				}

				break;
			}
			case Instruction::CMD_GO_TO_IF_FALSE:
			{
				int32_t blockId;
				if (state->readLevel == blockLevel)
					state->stream->read(&blockId);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					auto lastResult = module->getFrame(blockLevel).getLastIfResult();
					if (!lastResult)
					{
						auto position = module->getSavedPositions()[blockId];
						debug_log("Go to block: %d at position: %d", blockId, position);

						state->stream->seek(position);
					}
				}

				break;
			}
			case Instruction::CMD_PUSH_FUNCTION_CHAIN:
			{
				if (state->readLevel == blockLevel)
				{
					auto pos = (std::streamoff)state->stream->position() + 8;
					module->pushFunctionChain(pos);

					debug_log("Push position: %d", pos);
				}
				break;
			}
			case Instruction::CMD_POP_FUNCTION_CHAIN:
			{
				if (state->readLevel == blockLevel)
				{
					state->stream->seek(module->popFunctionChain());

					debug_log("Pop to position: %d", state->stream->position());
				}
				break;
			}
			case Instruction::CMD_CALL_NATIVE_FUNCTION:
			{
				int32_t blockId;
				if (state->readLevel == blockLevel)
					state->stream->read(&blockId);
				else
					state->stream->skip(sizeof(int32_t));

				int32_t numArgs;
				if (state->readLevel == blockLevel)
					state->stream->read(&numArgs);
				else
					state->stream->skip(sizeof(int32_t));

				int32_t varLen;
				state->stream->read(&varLen, sizeof(int32_t));

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Call native function: %s", varNameStr.c_str());

					if (callBindedFunction(varNameStr, numArgs))
					{
						auto obj = getObjectStack(StackType::STACK_FUNCTION_CALLBACK).top();
						getObjectStack(StackType::STACK_FUNCTION_CALLBACK).pop();
						module->getFrame(blockLevel).getEvaluator().loadObject(obj);
					}
					else
						Exception({ "Native function '" + varNameStr + "' not bound properly" }).display();
				}
				break;
			}
			case Instruction::CMD_CREATE_NATIVE_CLASS_INSTANCE:
			{
				int32_t classNameLen;
				state->stream->read(&classNameLen);

				char *className;
				std::string classNameStr;
				if (state->readLevel == blockLevel)
				{
					className = new char[classNameLen];
					state->stream->read(className, classNameLen);
					classNameStr = std::string(className);
					delete[] className;
				}
				else
					state->stream->skip(classNameLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Create native class instance: %s", classNameStr.c_str());

					createNativeObject(classNameStr);
				}

				break;
			}
			case Instruction::CMD_CALL_FUNCTION:
			{
				/*int32_t blockId;
				if (state->readLevel == blockLevel)
					state->stream->read(&blockId);
				else
					state->stream->skip(sizeof(int32_t));*/

				int32_t nameLen;
				state->stream->read(&nameLen);

				char *name;
				std::string nameStr;
				if (state->readLevel == blockLevel)
				{
					name = new char[nameLen];
					state->stream->read(name, nameLen);
					nameStr = std::string(name);
					delete[] name;
				}
				else
					state->stream->skip(nameLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Calling function: %s", nameStr.c_str());

					if (globalFunctions.find(nameStr) == globalFunctions.end())
						throw std::runtime_error("Function not defined");

					globalFunctions[nameStr]->invoke(state);

					/*debug_log("Push position: %d", state->stream->position());
					debug_log("Move to block: %d", blockId);

					module->pushFunctionChain((std::streamoff)state->stream->position());

					// move to block
					state->readLevel++;
					debug_log("Increase read level to: %d", state->readLevel);

					state->stream->seek(module->getSavedPositions()[blockId]);*/
				}

				break;
			}
			case Instruction::CMD_INVOKE_METHOD:
			{
				int32_t nameLen;
				state->stream->read(&nameLen);

				char *name;
				std::string nameStr;
				if (state->readLevel == blockLevel)
				{
					name = new char[nameLen];
					state->stream->read(name, nameLen);
					nameStr = std::string(name);
					delete[] name;
				}
				else
					state->stream->skip(nameLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Calling method function: %s", nameStr.c_str());

					auto object = module->getFrame(blockLevel).getEvaluator().getStack().top();
					object->invokeMethod(state, nameStr);
				}

				break;
			}
			case Instruction::CMD_LEAVE_FUNCTION:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Leave function");

					module->leaveFrame(blockLevel);
					blockLevel--;
					debug_log("Decrease block level to: %d", blockLevel);

					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);

					state->stream->seek(module->popFunctionChain());
					debug_log("Popping back to position: %d", state->stream->position());

					auto object = getObjectStack(StackType::STACK_FUNCTION_CALLBACK).top();
					getObjectStack(StackType::STACK_FUNCTION_CALLBACK).pop();
					module->getFrame(blockLevel).getEvaluator().loadObject(object);

					debug_log("Loaded variable from stack to level: %d, Value: '%s'",
						blockLevel, object->str().c_str());
				}
				break;
			}
			case Instruction::CMD_CREATE_VAR:
			{
				int32_t varType;
				if (state->readLevel == blockLevel)
					state->stream->read(&varType);
				else
					state->stream->skip(sizeof(int32_t));

				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				if (blockLevel == state->readLevel)
				{
					debug_log("Creating variable: %s", varNameStr.c_str());
					module->getFrame(blockLevel).createLocal(varNameStr);
				}

				break;
			}
			case Instruction::CMD_IF_STATEMENT:
			{
				if (state->readLevel == blockLevel)
				{
					auto &frame = module->getFrame(blockLevel);
					auto expr = frame.getEvaluator().getStack().top();

					bool val = (expr ? expr->cast<bool>() : false);

					frame.getEvaluator().getStack().pop();

					debug_log("If result: %s", (val ? "true" : "false"));

					frame.setLastIfResult(val);

					if (val)
					{
						state->readLevel++;
						debug_log("Increase read level to: %d", state->readLevel);
					}
				}

				break;
			}
			case Instruction::CMD_ELSE_STATEMENT:
			{
				if (state->readLevel == blockLevel)
				{
					bool lastResult = module->getFrame(blockLevel).getLastIfResult();

					if (!lastResult)
					{
						state->readLevel++;
						debug_log("Increase read level to: %d", state->readLevel);
					}
				}

				break;
			}
			case Instruction::CMD_LEAVE_BLOCK:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Leave block");

					module->leaveFrame(blockLevel);
					blockLevel--;
					debug_log("Decrease block level to: %d", blockLevel);

					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);
				}

				break;
			}
			case Instruction::CMD_LEAVE_IF_STATEMENT: // deprecated
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Leave if statement");

					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);
				}

				break;
			}
			case Instruction::CMD_LEAVE_ELSE_STATEMENT: // deprecated
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Leave else statement");

					state->readLevel--;
					debug_log("Decrease read level to: %d", state->readLevel);
				}

				break;
			}
			case Instruction::CMD_CLEAR_VAR:
			{
				int32_t varLen;
				state->stream->read(&varLen);

				if (state->readLevel == blockLevel)
				{
					char *varName = new char[varLen];
					state->stream->read(varName, varLen);
					std::string varNameStr(varName);
					delete[] varName;

					debug_log("Clear var: %s", varNameStr.c_str());

					int startLevel = blockLevel;
					bool found = false;

					while (startLevel >= -1)
					{
						auto &frame = module->getFrame(blockLevel);
						if (frame.hasLocal(varNameStr))
						{
							auto &obj = frame.getLocal(varNameStr);
							frame.clearLocal(obj);
							found = true;
							break;
						}

						startLevel--;
					}

					if (!found)
						throw std::runtime_error("Could not find object");
				}
				else
				{
					// skip ahead
					state->stream->skip(varLen);
				}

				break;
			}
			case Instruction::CMD_DELETE_VAR:
			{
				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Delete var: %s", varNameStr.c_str());
					module->getFrame(blockLevel).deleteLocal(varNameStr);
				}

				break;
			}
			case Instruction::CMD_LOOP_BREAK:
			{
				int32_t levelsToSkip;
				if (state->readLevel == blockLevel)
					state->stream->read(&levelsToSkip);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					debug_log("Loop break");
					module->getFrame(blockLevel - levelsToSkip).setLastIfResult(false);
					state->readLevel -= levelsToSkip;
				}

				break;
			}
			case Instruction::CMD_LOOP_CONTINUE:
			{
				int32_t levelsToSkip;
				if (state->readLevel == blockLevel)
					state->stream->read(&levelsToSkip);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					debug_log("Loop continue");
					module->getFrame(blockLevel - levelsToSkip).setLastIfResult(true);
					state->readLevel -= levelsToSkip;
				}

				break;
			}
			case Instruction::CMD_LOAD_INTEGER:
			{
				long value;
				if (state->readLevel == blockLevel)
					state->stream->read(&value);
				else
					state->stream->skip(sizeof(long));

				if (state->readLevel == blockLevel)
				{
					debug_log("Load integer: %d", value);
					module->getFrame(blockLevel).getEvaluator().loadInteger(value);
				}

				break;
			}
			case Instruction::CMD_LOAD_FLOAT:
			{
				double value;
				if (state->readLevel == blockLevel)
					state->stream->read(&value);
				else
					state->stream->skip(sizeof(double));

				if (state->readLevel == blockLevel)
				{
					debug_log("Load float: %f", value);
					module->getFrame(blockLevel).getEvaluator().loadFloat(value);
				}

				break;
			}
			case Instruction::CMD_LOAD_STRING:
			{
				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Load string: %s", varNameStr.c_str());
					module->getFrame(blockLevel).getEvaluator().loadString(varNameStr);
				}

				break;
			}
			case Instruction::CMD_LOAD_NULL:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Load null");
					module->getFrame(blockLevel).getEvaluator().loadNull();
				}
				break;
			}
			case Instruction::CMD_LOAD_VARIABLE:
			{
				int32_t varLen;
				state->stream->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (state->readLevel == blockLevel)
				{
					varName = new char[varLen];
					state->stream->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					state->stream->skip(varLen);

				if (state->readLevel == blockLevel)
				{
					debug_log("Loading variable: '%s'", varNameStr.c_str());


					int startLevel = blockLevel;
					bool found = false;

					while (startLevel >= -1)
					{
						auto &frame = module->getFrame(startLevel);
						if (frame.hasLocal(varNameStr))
						{
							auto &obj = frame.getLocal(varNameStr);
							module->getFrame(blockLevel).getEvaluator().loadObject(obj);

							debug_log("Loaded variable: '%s', Value: '%s', From level: %d, To level: %d",
								varNameStr.c_str(), obj->str().c_str(), startLevel, blockLevel);

							found = true;
							break;
						}

						startLevel--;
					}

					if (!found)
						throw std::runtime_error("Could not find object");
				}

				break;
			}
			case Instruction::CMD_OP_PUSH:
			{
				int32_t whichStack;
				if (state->readLevel == blockLevel)
					state->stream->read(&whichStack);
				else
					state->stream->skip(sizeof(int32_t));

				if (state->readLevel == blockLevel)
				{
					debug_log("Push result from level %d to object stack %d",
						blockLevel, whichStack);

					module->getFrame(blockLevel).getEvaluator().push(getObjectStack(whichStack));
				}

				break;
			}
			case Instruction::CMD_OP_CLEAR:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Clear expression");
					module->getFrame(blockLevel).getEvaluator().clear();
				}

				break;
			}
			case Instruction::CMD_OP_UNARY_NEG:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Unary -");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::u_minus);
				}

				break;
			}
			case Instruction::CMD_OP_UNARY_POS:
			{
				if (state->readLevel == blockLevel)
					debug_log("Unary +");

				break;
			}
			case Instruction::CMD_OP_UNARY_NOT:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Unary !");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::lognot);
				}

				break;
			}
			case Instruction::CMD_OP_ADD:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary +");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::add);
				}

				break;
			}
			case Instruction::CMD_OP_SUB:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary -");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::sub);
				}

				break;
			}
			case Instruction::CMD_OP_MUL:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary *");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::mul);
				}

				break;
			}
			case Instruction::CMD_OP_DIV:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary /");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::div);
				}

				break;
			}
			case Instruction::CMD_OP_MOD:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary %");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::mod);
				}

				break;
			}
			case Instruction::CMD_OP_AND:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary &&");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::logand);
				}

				break;
			}
			case Instruction::CMD_OP_OR:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary ||");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::logor);
				}

				break;
			}
			case Instruction::CMD_OP_EQL:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary ==");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::eql);
				}

				break;
			}
			case Instruction::CMD_OP_NEQL:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary !=");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::not_eql);
				}

				break;
			}
			case Instruction::CMD_OP_LT:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary <");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::less);
				}

				break;
			}
			case Instruction::CMD_OP_GT:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary >");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::greater);
				}

				break;
			}
			case Instruction::CMD_OP_LTE:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary <=");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::less_eql);
				}

				break;
			}
			case Instruction::CMD_OP_GTE:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary >=");
					module->getFrame(blockLevel).getEvaluator().operation(&Object::greater_eql);
				}

				break;
			}
			case Instruction::CMD_OP_ASSIGN:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary =");
					module->getFrame(blockLevel).getEvaluator().assign();
				}

				break;
			}
			case Instruction::CMD_OP_ADD_ASSIGN:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary +=");
					module->getFrame(blockLevel).getEvaluator().assign(&Object::add);
				}

				break;
			}
			case Instruction::CMD_OP_SUB_ASSIGN:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary -=");
					module->getFrame(blockLevel).getEvaluator().assign(&Object::sub);
				}

				break;
			}
			case Instruction::CMD_OP_MUL_ASSIGN:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary *=");
					module->getFrame(blockLevel).getEvaluator().assign(&Object::mul);
				}

				break;
			}
			case Instruction::CMD_OP_DIV_ASSIGN:
			{
				if (state->readLevel == blockLevel)
				{
					debug_log("Binary /=");
					module->getFrame(blockLevel).getEvaluator().assign(&Object::div);
				}

				break;
			}
			default:
				auto lastPos = (((unsigned long)state->stream->position()) - sizeof(Instruction));
				printf("Unrecognized instruction '%d' at position: %d\n", (int)ins, lastPos);
				state->stream = nullptr;
				return;
			}
		}

		void VM::exec()
		{
			Timer timer;
			timer.start();

			auto *module = new Module("main");
			state->module = module;

			while (state->stream != nullptr &&
				(state->stream->position() < state->stream->max()))
			{
				int32_t ins;
				state->stream->read(&ins);
				handleInstruction((Instruction)ins, module);
			}

			delete module;

			std::cout << "Execution completed in " << timer.elapsedTime() << "s\n";
		}

		ObjectStack &VM::getObjectStack(int id)
		{
			if (id >= objectStacks.size())
				throw std::out_of_range("Tried to access an unknown stack type");

			return objectStacks.at(id);
		}

		bool VM::callBindedFunction(const std::string &identifier, size_t numArgs)
		{
			for (auto &&item : nativeFunctions)
			{
				if (item.first == identifier)
				{
					if (item.second != nullptr && item.second->getNumParams() == numArgs)
					{
						item.second->f(getObjectStack(StackType::STACK_FUNCTION_PARAM),
							getObjectStack(StackType::STACK_FUNCTION_CALLBACK));

						return true;
					}
				}
			}

			return false;
		}

		bool VM::createNativeObject(const std::string &identifier)
		{
			return false;
		}
	}
}
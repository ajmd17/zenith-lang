#include "vm.h"

#include "../util/logger.h"
#include "../util/timer.h"

namespace zenith
{
	using namespace util;

	namespace runtime
	{
		VM::VM(ByteReader *byteReader)
		{
			this->byteReader = byteReader;

			for (int i = 0; i < 4; i++)
				objectStacks.push_back(ObjectStack());

			blockLevel = -1;
			readLevel = -1;
		}

		VM::~VM()
		{
			objectStacks.clear();

			/*int startLevel = blockLevel;
			while (startLevel >= -1)
				leaveFrame(startLevel--);*/
		}

		void VM::instruction(Instruction ins, std::unique_ptr<Module> &module)
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
				if (readLevel == blockLevel)
				{
					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);
				}

				module->leaveFrame(blockLevel);
				blockLevel--;

				debug_log("Decrease block level to: %d", blockLevel);
				break;
			}
			case Instruction::CMD_INC_READ_LEVEL:
			{
				if (readLevel == blockLevel)
				{
					readLevel++;
					debug_log("Increase read level to: %d", readLevel);
				}
				break;
			}
			case Instruction::CMD_DEC_READ_LEVEL:
			{
				if (readLevel == blockLevel)
				{
					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);
				}
				break;
			}
			case Instruction::CMD_STACK_POP_OBJECT:
			{
				// which stack to use
				int32_t whichStack;
				if (readLevel == blockLevel)
					byteReader->read(&whichStack);
				else
					byteReader->skip(sizeof(int32_t));

				// load string for variable name
				int32_t varLen;
				byteReader->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				// pop result into variable
				if (readLevel == blockLevel)
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
							ValuePtr &valuePtr = frame.getLocal(varNameStr);
							valuePtr = getObjectStack(whichStack).top();
							getObjectStack(whichStack).pop();

							debug_log("Set variable '%s' to value: '%s'",
								varNameStr.c_str(), valuePtr->str().c_str());

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
				byteReader->read(&blockId);

				int32_t blockType;
				byteReader->read(&blockType);

				int32_t parentId;
				byteReader->read(&parentId);

				uint64_t blockPos;
				byteReader->read(&blockPos);

				module->getSavedPositions()[blockId] = blockPos;

				debug_log("Create block: %d at position: %d", blockId, blockPos);

				break;
			}
			case Instruction::CMD_GO_TO_BLOCK:
			{
				int32_t blockId;
				if (readLevel == blockLevel)
					byteReader->read(&blockId);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					auto position = module->getSavedPositions()[blockId];
					debug_log("Go to block: %d at position: %d", blockId, position);

					byteReader->seek(position);
				}

				break;
			}
			case Instruction::CMD_GO_TO_IF_TRUE:
			{
				int32_t blockId;
				if (readLevel == blockLevel)
					byteReader->read(&blockId);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					auto lastResult = module->getFrame(blockLevel).getLastIfResult();
					if (lastResult)
					{
						auto position = module->getSavedPositions()[blockId];
						debug_log("Go to block: %d at position: %d", blockId, position);

						byteReader->seek(position);
					}
				}

				break;
			}
			case Instruction::CMD_GO_TO_IF_FALSE:
			{
				int32_t blockId;
				if (readLevel == blockLevel)
					byteReader->read(&blockId);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					auto lastResult = module->getFrame(blockLevel).getLastIfResult();
					if (!lastResult)
					{
						auto position = module->getSavedPositions()[blockId];
						debug_log("Go to block: %d at position: %d", blockId, position);

						byteReader->seek(position);
					}
				}

				break;
			}
			case Instruction::CMD_PUSH_FUNCTION_CHAIN:
			{
				if (readLevel == blockLevel)
				{
					auto pos = (std::streamoff)byteReader->position() + 8;
					module->pushFunctionChain(pos);

					debug_log("Push position: %d", pos);
				}
				break;
			}
			case Instruction::CMD_POP_FUNCTION_CHAIN:
			{
				if (readLevel == blockLevel)
				{
					byteReader->seek(module->popFunctionChain());

					debug_log("Pop to position: %d", byteReader->position());
				}
				break;
			}
			case Instruction::CMD_CALL_NATIVE_FUNCTION:
			{
				int32_t blockId;
				if (readLevel == blockLevel)
					byteReader->read(&blockId);
				else
					byteReader->skip(sizeof(int32_t));

				int32_t numArgs;
				if (readLevel == blockLevel)
					byteReader->read(&numArgs);
				else
					byteReader->skip(sizeof(int32_t));

				int32_t varLen;
				byteReader->read(&varLen, sizeof(int32_t));

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				if (readLevel == blockLevel)
				{
					debug_log("Call native function: %s", varNameStr.c_str());

					if (callBindedFunction(varNameStr, numArgs))
					{
						ValuePtr val = getObjectStack(StackType::STACK_FUNCTION_CALLBACK).top();
						getObjectStack(StackType::STACK_FUNCTION_CALLBACK).pop();
						module->getFrame(blockLevel).getEvaluator().loadVariable(val);
					}
					else
						Exception({ "Native function '" + varNameStr + "' not bound properly" }).display();
				}
				break;
			}
			case Instruction::CMD_CREATE_NATIVE_CLASS_INSTANCE:
			{
				int32_t classNameLen;
				byteReader->read(&classNameLen);

				char *className;
				std::string classNameStr;
				if (readLevel == blockLevel)
				{
					className = new char[classNameLen];
					byteReader->read(className, classNameLen);
					classNameStr = std::string(className);
					delete[] className;
				}
				else
					byteReader->skip(classNameLen);

				if (readLevel == blockLevel)
				{
					debug_log("Create native class instance: %s", classNameStr.c_str());

					createNativeObject(classNameStr);
				}

				break;
			}
			case Instruction::CMD_CALL_FUNCTION:
			{
				int32_t blockId;
				if (readLevel == blockLevel)
					byteReader->read(&blockId);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					debug_log("Push position: %d", byteReader->position());
					debug_log("Move to block: %d", blockId);

					module->pushFunctionChain((std::streamoff)byteReader->position());

					// move to block
					readLevel++;
					debug_log("Increase read level to: %d", readLevel);

					byteReader->seek(module->getSavedPositions()[blockId]);
				}

				break;
			}
			case Instruction::CMD_LEAVE_FUNCTION:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Leave function");

					module->leaveFrame(blockLevel);
					blockLevel--;
					debug_log("Decrease block level to: %d", blockLevel);

					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);

					byteReader->seek(module->popFunctionChain());
					debug_log("Popping back to position: %d", byteReader->position());

					ValuePtr val = getObjectStack(StackType::STACK_FUNCTION_CALLBACK).top();
					getObjectStack(StackType::STACK_FUNCTION_CALLBACK).pop();
					module->getFrame(blockLevel).getEvaluator().loadVariable(val);

					debug_log("Loaded variable from stack to level: %d, Value: '%s'",
						blockLevel, val->str().c_str());
				}
				break;
			}
			case Instruction::CMD_CREATE_VAR:
			{
				int32_t varType;
				if (readLevel == blockLevel)
					byteReader->read(&varType);
				else
					byteReader->skip(sizeof(int32_t));

				int32_t varLen;
				byteReader->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				if (blockLevel == readLevel)
				{
					debug_log("Creating variable: %s", varNameStr.c_str());
					module->getFrame(blockLevel).createLocal(varNameStr);
				}

				break;
			}
			case Instruction::CMD_IF_STATEMENT:
			{
				if (readLevel == blockLevel)
				{
					auto &frame = module->getFrame(blockLevel);
					ValuePtr expr = frame.getEvaluator().getStack().top();

					bool val;
					if (expr != nullptr)
						val = expr->getData<bool>();
					else
						val = false; // null values convert to false

					frame.getEvaluator().getStack().pop();

					debug_log("If result: %s", (val ? "true" : "false"));

					frame.setLastIfResult(val);

					if (val)
					{
						readLevel++;
						debug_log("Increase read level to: %d", readLevel);
					}
				}

				break;
			}
			case Instruction::CMD_ELSE_STATEMENT:
			{
				if (readLevel == blockLevel)
				{
					bool lastResult = module->getFrame(blockLevel).getLastIfResult();

					if (!lastResult)
					{
						readLevel++;
						debug_log("Increase read level to: %d", readLevel);
					}
				}

				break;
			}
			case Instruction::CMD_LEAVE_BLOCK:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Leave block");

					module->leaveFrame(blockLevel);
					blockLevel--;
					debug_log("Decrease block level to: %d", blockLevel);

					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);
				}

				break;
			}
			case Instruction::CMD_LEAVE_IF_STATEMENT: // deprecated
			{
				if (readLevel == blockLevel)
				{
					debug_log("Leave if statement");

					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);
				}

				break;
			}
			case Instruction::CMD_LEAVE_ELSE_STATEMENT: // deprecated
			{
				if (readLevel == blockLevel)
				{
					debug_log("Leave else statement");

					readLevel--;
					debug_log("Decrease read level to: %d", readLevel);
				}

				break;
			}
			case Instruction::CMD_CLEAR_VAR:
			{
				int32_t varLen;
				byteReader->read(&varLen);

				if (readLevel == blockLevel)
				{
					char *varName = new char[varLen];
					byteReader->read(varName, varLen);
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
							ValuePtr &valuePtr = frame.getLocal(varNameStr);
							frame.clearLocal(valuePtr);
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
					byteReader->skip(varLen);
				}

				break;
			}
			case Instruction::CMD_DELETE_VAR:
			{
				int32_t varLen;
				byteReader->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				if (readLevel == blockLevel)
				{
					debug_log("Delete var: %s", varNameStr.c_str());
					module->getFrame(blockLevel).deleteLocal(varNameStr);
				}

				break;
			}
			case Instruction::CMD_LOOP_BREAK:
			{
				int32_t levelsToSkip;
				if (readLevel == blockLevel)
					byteReader->read(&levelsToSkip);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					debug_log("Loop break");
					module->getFrame(blockLevel - levelsToSkip).setLastIfResult(false);
					readLevel -= levelsToSkip;
				}

				break;
			}
			case Instruction::CMD_LOOP_CONTINUE:
			{
				int32_t levelsToSkip;
				if (readLevel == blockLevel)
					byteReader->read(&levelsToSkip);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					debug_log("Loop continue");
					module->getFrame(blockLevel - levelsToSkip).setLastIfResult(true);
					readLevel -= levelsToSkip;
				}

				break;
			}
			case Instruction::CMD_LOAD_INTEGER:
			{
				long value;
				if (readLevel == blockLevel)
					byteReader->read(&value);
				else
					byteReader->skip(sizeof(long));

				if (readLevel == blockLevel)
				{
					debug_log("Load integer: %d", value);
					module->getFrame(blockLevel).getEvaluator().loadInteger(value);
				}

				break;
			}
			case Instruction::CMD_LOAD_FLOAT:
			{
				double value;
				if (readLevel == blockLevel)
					byteReader->read(&value);
				else
					byteReader->skip(sizeof(double));

				if (readLevel == blockLevel)
				{
					debug_log("Load float: %f", value);
					module->getFrame(blockLevel).getEvaluator().loadFloat(value);
				}

				break;
			}
			case Instruction::CMD_LOAD_STRING:
			{
				int32_t varLen;
				byteReader->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				if (readLevel == blockLevel)
				{
					debug_log("Load string: %s", varNameStr.c_str());
					module->getFrame(blockLevel).getEvaluator().loadString(varNameStr);
				}

				break;
			}
			case Instruction::CMD_LOAD_NULL:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Load null");
					module->getFrame(blockLevel).getEvaluator().loadNull();
				}
				break;
			}
			case Instruction::CMD_LOAD_VARIABLE:
			{
				int32_t varLen;
				byteReader->read(&varLen);

				char *varName;
				std::string varNameStr;
				if (readLevel == blockLevel)
				{
					varName = new char[varLen];
					byteReader->read(varName, varLen);
					varNameStr = std::string(varName);
					delete[] varName;
				}
				else
					byteReader->skip(varLen);

				if (readLevel == blockLevel)
				{
					debug_log("Loading variable: '%s'", varNameStr.c_str());


					int startLevel = blockLevel;
					bool found = false;

					while (startLevel >= -1)
					{
						auto &frame = module->getFrame(startLevel);
						if (frame.hasLocal(varNameStr))
						{
							ValuePtr &valuePtr = frame.getLocal(varNameStr);
							module->getFrame(blockLevel).getEvaluator().loadVariable(valuePtr);

							debug_log("Loaded variable: '%s', Value: '%s', From level: %d, To level: %d",
								varNameStr.c_str(), valuePtr->str().c_str(), startLevel, blockLevel);

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
				if (readLevel == blockLevel)
					byteReader->read(&whichStack);
				else
					byteReader->skip(sizeof(int32_t));

				if (readLevel == blockLevel)
				{
					debug_log("Push result from level %d to object stack %d",
						blockLevel, whichStack);

					module->getFrame(blockLevel).getEvaluator().push(getObjectStack(whichStack));
				}

				break;
			}
			case Instruction::CMD_OP_CLEAR:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Clear expression");
					module->getFrame(blockLevel).getEvaluator().clear();
				}

				break;
			}
			case Instruction::CMD_OP_UNARY_NEG:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Unary -");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::unaryMinus);
				}

				break;
			}
			case Instruction::CMD_OP_UNARY_POS:
			{
				if (readLevel == blockLevel)
					debug_log("Unary +");

				break;
			}
			case Instruction::CMD_OP_UNARY_NOT:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Unary !");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::unaryNot);
				}

				break;
			}
			case Instruction::CMD_OP_ADD:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary +");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::add);
				}

				break;
			}
			case Instruction::CMD_OP_SUB:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary -");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::subtract);
				}

				break;
			}
			case Instruction::CMD_OP_MUL:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary *");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::multiply);
				}

				break;
			}
			case Instruction::CMD_OP_DIV:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary /");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::divide);
				}

				break;
			}
			case Instruction::CMD_OP_MOD:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary %");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::modulus);
				}

				break;
			}
			case Instruction::CMD_OP_AND:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary &&");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::logicalAnd);
				}

				break;
			}
			case Instruction::CMD_OP_OR:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary ||");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::logicalOr);
				}

				break;
			}
			case Instruction::CMD_OP_EQL:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary ==");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::isEqualTo);
				}

				break;
			}
			case Instruction::CMD_OP_NEQL:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary !=");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::notEqualTo);
				}

				break;
			}
			case Instruction::CMD_OP_LT:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary <");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::lessThan);
				}

				break;
			}
			case Instruction::CMD_OP_GT:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary >");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::greaterThan);
				}

				break;
			}
			case Instruction::CMD_OP_LTE:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary <=");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::lessOrEqualTo);
				}

				break;
			}
			case Instruction::CMD_OP_GTE:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary >=");
					module->getFrame(blockLevel).getEvaluator().operation(&Value::greaterOrEqualTo);
				}

				break;
			}
			case Instruction::CMD_OP_ASSIGN:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary =");
					module->getFrame(blockLevel).getEvaluator().assign();
				}

				break;
			}
			case Instruction::CMD_OP_ADD_ASSIGN:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary +=");
					module->getFrame(blockLevel).getEvaluator().assign(&Value::add);
				}

				break;
			}
			case Instruction::CMD_OP_SUB_ASSIGN:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary -=");
					module->getFrame(blockLevel).getEvaluator().assign(&Value::subtract);
				}

				break;
			}
			case Instruction::CMD_OP_MUL_ASSIGN:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary *=");
					module->getFrame(blockLevel).getEvaluator().assign(&Value::multiply);
				}

				break;
			}
			case Instruction::CMD_OP_DIV_ASSIGN:
			{
				if (readLevel == blockLevel)
				{
					debug_log("Binary /=");
					module->getFrame(blockLevel).getEvaluator().assign(&Value::divide);
				}

				break;
			}
			default:
				auto lastPos = (((unsigned long)byteReader->position()) - sizeof(Instruction));
				printf("Unrecognized instruction '%d' at position: %d\n", (int)ins, lastPos);
				byteReader = nullptr;
				return;
			}
		}

		void VM::exec()
		{
			Timer timer;
			timer.start();

			auto module = std::make_unique<Module>("main");

			while (byteReader != nullptr &&
				(byteReader->position() < byteReader->max()))
			{
				int32_t ins;
				byteReader->read(&ins);
				instruction((Instruction)ins, module);
			}

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
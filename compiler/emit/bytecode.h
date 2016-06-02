#ifndef __ZENITH_COMPILER_BYTECODE_H__
#define __ZENITH_COMPILER_BYTECODE_H__

#include <cstdint>
#include <vector>
#include <stack>
#include <string>
#include <iostream>
#include <memory>

#include "../../enums.h"

namespace zenith
{
	namespace compiler
	{
		struct BytecodeCommand
		{
			Instruction command = Instruction::CMD_NONE;

			BytecodeCommand(Instruction cmd = Instruction::CMD_NONE)
			{
				this->command = cmd;
			}
		};

		typedef std::vector<std::shared_ptr<BytecodeCommand>> BytecodeCommandList;

		struct IncreaseBlockLevel : public BytecodeCommand
		{
			IncreaseBlockLevel() : BytecodeCommand(Instruction::CMD_INC_BLOCK_LEVEL) { }
		};

		struct DecreaseBlockLevel : public BytecodeCommand
		{
			DecreaseBlockLevel() : BytecodeCommand(Instruction::CMD_DEC_BLOCK_LEVEL) { }
		};

		struct IncreaseReadLevel : public BytecodeCommand
		{
			IncreaseReadLevel() : BytecodeCommand(Instruction::CMD_INC_READ_LEVEL) { }
		};

		struct DecreaseReadLevel : public BytecodeCommand
		{
			DecreaseReadLevel() : BytecodeCommand(Instruction::CMD_DEC_READ_LEVEL) { }
		};

		struct LeaveBlock : public BytecodeCommand
		{
			LeaveBlock() : BytecodeCommand(Instruction::CMD_LEAVE_BLOCK) { }
		};

		struct StackPopObject : public BytecodeCommand
		{
			std::string varName;
			int whichStack;

			StackPopObject(const std::string &varName, int whichStack) : BytecodeCommand(Instruction::CMD_STACK_POP_OBJECT)
			{
				this->varName = varName;
				this->whichStack = whichStack;
			}
		};

		struct CreateBlock : public BytecodeCommand
		{
			BlockType blockType;
			unsigned int blockId;
			unsigned int parentId;

			CreateBlock(BlockType blockType, unsigned int blockId, unsigned int parentId) : BytecodeCommand(Instruction::CMD_CREATE_BLOCK)
			{
				this->blockType = blockType;
				this->blockId = blockId;
				this->parentId = parentId;
			}
		};

		struct GoToBlock : public BytecodeCommand
		{
			unsigned int blockId;

			GoToBlock(unsigned int blockId) : BytecodeCommand(Instruction::CMD_GO_TO_BLOCK)
			{
				this->blockId = blockId;
			}
		};

		struct GoToIfTrue : public BytecodeCommand
		{
			unsigned int blockId;

			GoToIfTrue(unsigned int blockId) : BytecodeCommand(Instruction::CMD_GO_TO_IF_TRUE)
			{
				this->blockId = blockId;
			}
		};

		struct GoToIfFalse : public BytecodeCommand
		{
			unsigned int blockId;

			GoToIfFalse(unsigned int blockId) : BytecodeCommand(Instruction::CMD_GO_TO_IF_FALSE)
			{
				this->blockId = blockId;
			}
		};

		struct CallFunction : public BytecodeCommand
		{
			/*unsigned int blockId;*/
			std::string functionName;

			CallFunction(/*unsigned int blockId*/ const std::string &name) : BytecodeCommand(Instruction::CMD_CALL_FUNCTION)
			{
				//this->blockId = blockId;
				this->functionName = name;
			}
		};

		struct InvokeMethod : public BytecodeCommand
		{
			std::string functionName;

			InvokeMethod(const std::string &name) : BytecodeCommand(Instruction::CMD_INVOKE_METHOD)
			{
				this->functionName = name;
			}
		};

		struct CallNativeFunction : public BytecodeCommand
		{
			std::string fnName;
			unsigned int blockId;
			unsigned int numArgs;

			CallNativeFunction(const std::string &fnName, unsigned int blockId, unsigned int numArgs) : BytecodeCommand(Instruction::CMD_CALL_NATIVE_FUNCTION)
			{
				this->fnName = fnName;
				this->blockId = blockId;
				this->numArgs = numArgs;
			}
		};

		struct CreateNativeClassInstance : public BytecodeCommand
		{
			std::string className;

			CreateNativeClassInstance(const std::string &className) : BytecodeCommand(Instruction::CMD_CREATE_NATIVE_CLASS_INSTANCE)
			{
				this->className = className;
			}
		};

		struct CreateFunction : public BytecodeCommand
		{
			std::string functionName;

			CreateFunction(const std::string &functionName) : BytecodeCommand(Instruction::CMD_CREATE_FUNCTION)
			{
				this->functionName = functionName;
			}
		};

		struct LeaveFunction : public BytecodeCommand
		{
			LeaveFunction() : BytecodeCommand(Instruction::CMD_LEAVE_FUNCTION) { }
		};

		struct PushFunctionChain : public BytecodeCommand
		{
			PushFunctionChain() : BytecodeCommand(Instruction::CMD_PUSH_FUNCTION_CHAIN) { }
		};

		struct PopFunctionChain : public BytecodeCommand
		{
			PopFunctionChain() : BytecodeCommand(Instruction::CMD_POP_FUNCTION_CHAIN) { }
		};

		struct IfStatement : public BytecodeCommand
		{
			IfStatement() : BytecodeCommand(Instruction::CMD_IF_STATEMENT)
			{
			}
		};

		struct ElseStatement : public BytecodeCommand
		{
			ElseStatement() : BytecodeCommand(Instruction::CMD_ELSE_STATEMENT) { }
		};

		struct LeaveIfStatement : public BytecodeCommand
		{
			LeaveIfStatement() : BytecodeCommand(Instruction::CMD_LEAVE_IF_STATEMENT) { }
		};

		struct LeaveElseStatement : public BytecodeCommand
		{
			LeaveElseStatement() : BytecodeCommand(Instruction::CMD_LEAVE_ELSE_STATEMENT) { }
		};

		struct VarCreate : public BytecodeCommand
		{
			VarType varType;
			std::string varName;

			VarCreate(VarType varType, const std::string &varName) : BytecodeCommand(Instruction::CMD_CREATE_VAR)
			{
				this->varType = varType;
				this->varName = varName;
			}
		};

		struct VarAddProperty : public BytecodeCommand
		{
			std::string varName;
			std::string propertyName;

			VarAddProperty(const std::string &varName, const std::string &propertyName) : BytecodeCommand(Instruction::CMD_ADD_PROPERTY)
			{
				this->varName = varName;
				this->propertyName = propertyName;
			}
		};

		struct VarPushProperty : public BytecodeCommand
		{
			std::string varName;
			std::string propertyName;

			VarPushProperty(const std::string &varName, const std::string &propertyName) : BytecodeCommand(Instruction::CMD_PUSH_PROPERTY)
			{
				this->varName = varName;
				this->propertyName = propertyName;
			}
		};

		struct DeleteObject : public BytecodeCommand
		{
			std::string varName;

			DeleteObject(const std::string &varName) : BytecodeCommand(Instruction::CMD_CLEAR_VAR)
			{
				this->varName = varName;
			}
		};

		struct RemoveReference : public BytecodeCommand
		{
			std::string varName;

			RemoveReference(const std::string &varName) : BytecodeCommand(Instruction::CMD_DELETE_VAR)
			{
				this->varName = varName;
			}
		};

		struct LoopBreak : public BytecodeCommand
		{
			int levelsToSkip;

			LoopBreak(int levelsToSkip) : BytecodeCommand(Instruction::CMD_LOOP_BREAK)
			{
				this->levelsToSkip = levelsToSkip;
			}
		};

		struct LoopContinue : public BytecodeCommand
		{
			int levelsToSkip;

			LoopContinue(int levelsToSkip) : BytecodeCommand(Instruction::CMD_LOOP_CONTINUE)
			{
				this->levelsToSkip = levelsToSkip;
			}
		};

		struct LoadInteger : public BytecodeCommand
		{
			long val;

			LoadInteger(long intValue) : BytecodeCommand(Instruction::CMD_LOAD_INTEGER)
			{
				val = intValue;
			}
		};

		struct LoadFloat : public BytecodeCommand
		{
			double val;

			LoadFloat(double floatValue) : BytecodeCommand(Instruction::CMD_LOAD_FLOAT)
			{
				val = floatValue;
			}
		};

		struct LoadVariable : public BytecodeCommand
		{
			std::string val;

			LoadVariable(const std::string &name) : BytecodeCommand(Instruction::CMD_LOAD_VARIABLE)
			{
				val = name;
			}
		};

		struct LoadString : public BytecodeCommand
		{
			std::string val;

			LoadString(const std::string &strValue) : BytecodeCommand(Instruction::CMD_LOAD_STRING)
			{
				val = strValue;
			}
		};

		struct LoadNull : public BytecodeCommand
		{
			LoadNull() : BytecodeCommand(Instruction::CMD_LOAD_NULL) { }
		};

		struct OpPush : public BytecodeCommand
		{
			int whichStack;
			OpPush(int stack) : BytecodeCommand(Instruction::CMD_OP_PUSH)
			{
				whichStack = stack;
			}
		};

		struct OpClear : public BytecodeCommand
		{
			OpClear() : BytecodeCommand(Instruction::CMD_OP_CLEAR) { }
		};

		struct OpBinaryAdd : public BytecodeCommand
		{
			OpBinaryAdd() : BytecodeCommand(Instruction::CMD_OP_ADD) { }
		};

		struct OpBinarySub : public BytecodeCommand
		{
			OpBinarySub() : BytecodeCommand(Instruction::CMD_OP_SUB) { }
		};

		struct OpBinaryMul : public BytecodeCommand
		{
			OpBinaryMul() : BytecodeCommand(Instruction::CMD_OP_MUL) { }
		};

		struct OpBinaryDiv : public BytecodeCommand
		{
			OpBinaryDiv() : BytecodeCommand(Instruction::CMD_OP_DIV) { }
		};

		struct OpBinaryMod : public BytecodeCommand
		{
			OpBinaryMod() : BytecodeCommand(Instruction::CMD_OP_MOD) { }
		};

		struct OpBinaryPow : public BytecodeCommand
		{
			OpBinaryPow() : BytecodeCommand(Instruction::CMD_OP_POW) { }
		};

		struct OpUnaryNot : public BytecodeCommand
		{
			OpUnaryNot() : BytecodeCommand(Instruction::CMD_OP_UNARY_NOT) { }
		};

		struct OpUnaryMinus : public BytecodeCommand
		{
			OpUnaryMinus() : BytecodeCommand(Instruction::CMD_OP_UNARY_NEG) { }
		};

		struct OpUnaryPos : public BytecodeCommand
		{
			OpUnaryPos() : BytecodeCommand(Instruction::CMD_OP_UNARY_POS) { }
		};

		struct OpBinaryAnd : public BytecodeCommand
		{
			OpBinaryAnd() : BytecodeCommand(Instruction::CMD_OP_AND) { }
		};

		struct OpBinaryOr : public BytecodeCommand
		{
			OpBinaryOr() : BytecodeCommand(Instruction::CMD_OP_OR) { }
		};

		struct OpBinaryEql : public BytecodeCommand
		{
			OpBinaryEql() : BytecodeCommand(Instruction::CMD_OP_EQL) { }
		};

		struct OpBinaryNotEql : public BytecodeCommand
		{
			OpBinaryNotEql() : BytecodeCommand(Instruction::CMD_OP_NEQL) { }
		};

		struct OpBinaryLT : public BytecodeCommand
		{
			OpBinaryLT() : BytecodeCommand(Instruction::CMD_OP_LT) { }
		};

		struct OpBinaryGT : public BytecodeCommand
		{
			OpBinaryGT() : BytecodeCommand(Instruction::CMD_OP_GT) { }
		};

		struct OpBinaryLTE : public BytecodeCommand
		{
			OpBinaryLTE() : BytecodeCommand(Instruction::CMD_OP_LTE) { }
		};

		struct OpBinaryGTE : public BytecodeCommand
		{
			OpBinaryGTE() : BytecodeCommand(Instruction::CMD_OP_GTE) { }
		};

		struct OpBinaryAssign : public BytecodeCommand
		{
			OpBinaryAssign() : BytecodeCommand(Instruction::CMD_OP_ASSIGN) { }
		};

		struct OpBinaryAddAssign : public BytecodeCommand
		{
			OpBinaryAddAssign() : BytecodeCommand(Instruction::CMD_OP_ADD_ASSIGN) { }
		};

		struct OpBinarySubAssign : public BytecodeCommand
		{
			OpBinarySubAssign() : BytecodeCommand(Instruction::CMD_OP_SUB_ASSIGN) { }
		};

		struct OpBinaryMulAssign : public BytecodeCommand
		{
			OpBinaryMulAssign() : BytecodeCommand(Instruction::CMD_OP_MUL_ASSIGN) { }
		};

		struct OpBinaryDivAssign : public BytecodeCommand
		{
			OpBinaryDivAssign() : BytecodeCommand(Instruction::CMD_OP_DIV_ASSIGN) { }
		};
	}
}

#endif
///
/// @file LoadInstruction.cpp
/// @brief 间接读指令
/// @author Jiu-Zz
///
#include "LoadInstruction.h"
#include "VoidType.h"

LoadInstruction::LoadInstruction(Function * _func, Value * addr, Type * _type)
	: Instruction(_func, IRInstOperator::IRINST_OP_LOAD, _type)
{
	addOperand(addr);
}

void LoadInstruction::toString(std::string & str)
{
	auto addr = getOperand(0);
	str = getIRName() + " = *" + addr->getIRName();
}

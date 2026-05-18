///
/// @file StoreInstruction.cpp
/// @brief 间接写指令
/// @author Jiu-Zz
///
#include "StoreInstruction.h"
#include "VoidType.h"

StoreInstruction::StoreInstruction(Function * _func, Value * addr, Value * src)
	: Instruction(_func, IRInstOperator::IRINST_OP_STORE, VoidType::getType())
{
	addOperand(addr);
	addOperand(src);
}

void StoreInstruction::toString(std::string & str)
{
	auto addr = getOperand(0);
	auto src = getOperand(1);
	str = "*" + addr->getIRName() + " = " + src->getIRName();
}

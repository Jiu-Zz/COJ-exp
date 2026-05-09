///
/// @file CondBranchInstruction.cpp
/// @brief 条件分支指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2026-05-09
///
/// @copyright Copyright (c) 2024
///
#include "VoidType.h"

#include "CondBranchInstruction.h"

CondBranchInstruction::CondBranchInstruction(
	Function * _func, Value * _cond, Instruction * _trueTarget, Instruction * _falseTarget)
	: Instruction(_func, IRInstOperator::IRINST_OP_CBR, VoidType::getType())
{
	addOperand(_cond);
	trueTarget = static_cast<LabelInstruction *>(_trueTarget);
	falseTarget = static_cast<LabelInstruction *>(_falseTarget);
}

void CondBranchInstruction::toString(std::string & str)
{
	Value * cond = getOperand(0);
	str = "bc " + cond->getIRName() + ", label " + trueTarget->getIRName() + ", label " + falseTarget->getIRName();
}

LabelInstruction * CondBranchInstruction::getTrueTarget() const
{
	return trueTarget;
}

LabelInstruction * CondBranchInstruction::getFalseTarget() const
{
	return falseTarget;
}

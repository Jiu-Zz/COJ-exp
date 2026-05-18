///
/// @file LoadInstruction.h
/// @brief 间接读指令
/// @author Jiu-Zz
///
#pragma once

#include "Instruction.h"

class LoadInstruction : public Instruction {

public:
	LoadInstruction(Function * _func, Value * addr, Type * _type);

	void toString(std::string & str) override;
};

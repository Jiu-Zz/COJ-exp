///
/// @file StoreInstruction.h
/// @brief 间接写指令
/// @author Jiu-Zz
///
#pragma once

#include "Instruction.h"

class StoreInstruction : public Instruction {

public:
	StoreInstruction(Function * _func, Value * addr, Value * src);

	void toString(std::string & str) override;
};

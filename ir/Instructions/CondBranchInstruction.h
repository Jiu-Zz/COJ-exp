///
/// @file CondBranchInstruction.h
/// @brief 条件分支指令
///
/// @author zenglj (zenglj@live.com)
/// @version 1.0
/// @date 2026-05-09
///
/// @copyright Copyright (c) 2024
///
#pragma once

#include <string>

#include "Function.h"
#include "Instruction.h"
#include "LabelInstruction.h"

///
/// @brief 条件分支指令
///
class CondBranchInstruction final : public Instruction {

public:
	///
	/// @brief 条件分支指令构造函数
	/// @param _func 所属函数
	/// @param _cond 条件值
	/// @param _trueTarget 条件为真时跳转的标签
	/// @param _falseTarget 条件为假时跳转的标签
	///
	CondBranchInstruction(Function * _func, Value * _cond, Instruction * _trueTarget, Instruction * _falseTarget);

	/// @brief 转换成字符串
	void toString(std::string & str) override;

	/// @brief 获取真分支标签
	[[nodiscard]] LabelInstruction * getTrueTarget() const;

	/// @brief 获取假分支标签
	[[nodiscard]] LabelInstruction * getFalseTarget() const;

private:
	LabelInstruction * trueTarget;
	LabelInstruction * falseTarget;
};

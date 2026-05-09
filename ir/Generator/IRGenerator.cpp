///
/// @file IRGenerator.cpp
/// @brief AST遍历产生线性IR的源文件
/// @author zenglj (zenglj@live.com)
/// @version 1.1
/// @date 2024-11-23
///
/// @copyright Copyright (c) 2024
///
/// @par 修改日志:
/// <table>
/// <tr><th>Date       <th>Version <th>Author  <th>Description
/// <tr><td>2024-09-29 <td>1.0     <td>zenglj  <td>新建
/// <tr><td>2024-11-23 <td>1.1     <td>zenglj  <td>表达式版增强
/// </table>
///
#include <cstdint>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "AST.h"
#include "Common.h"
#include "Function.h"
#include "IRCode.h"
#include "IRGenerator.h"
#include "Module.h"
#include "EntryInstruction.h"
#include "CondBranchInstruction.h"
#include "LabelInstruction.h"
#include "ExitInstruction.h"
#include "FuncCallInstruction.h"
#include "BinaryInstruction.h"
#include "MoveInstruction.h"
#include "GotoInstruction.h"

namespace {

Function * registerFunctionSignature(Module * module, ast_node * node)
{
	if (!module || !node || node->node_type != ast_operator_type::AST_OP_FUNC_DEF) {
		return nullptr;
	}

	ast_node * typeNode = node->sons[0];
	ast_node * nameNode = node->sons[1];
	ast_node * paramsNode = node->sons[2];

	std::vector<FormalParam *> params;
	if (paramsNode) {
		for (auto paramNode: paramsNode->sons) {
			if (!paramNode) {
				continue;
			}

			auto * param = new FormalParam(paramNode->type, paramNode->name);
			params.push_back(param);
		}
	}

	return module->newFunction(nameNode->name, typeNode->type, params, false);
}

} // namespace

/// @brief 构造函数
/// @param _root AST的根
/// @param _module 符号表
IRGenerator::IRGenerator(ast_node * _root, Module * _module) : root(_root), module(_module)
{
	/* 叶子节点 */
	ast2ir_handlers[ast_operator_type::AST_OP_LEAF_LITERAL_UINT] = &IRGenerator::ir_leaf_node_uint;
	ast2ir_handlers[ast_operator_type::AST_OP_LEAF_VAR_ID] = &IRGenerator::ir_leaf_node_var_id;
	ast2ir_handlers[ast_operator_type::AST_OP_LEAF_TYPE] = &IRGenerator::ir_leaf_node_type;

	/* 表达式运算， 加减 */
	ast2ir_handlers[ast_operator_type::AST_OP_SUB] = &IRGenerator::ir_sub;
	ast2ir_handlers[ast_operator_type::AST_OP_ADD] = &IRGenerator::ir_add;
	ast2ir_handlers[ast_operator_type::AST_OP_MUL] = &IRGenerator::ir_mul;
	ast2ir_handlers[ast_operator_type::AST_OP_DIV] = &IRGenerator::ir_div;
	ast2ir_handlers[ast_operator_type::AST_OP_MOD] = &IRGenerator::ir_mod;
	ast2ir_handlers[ast_operator_type::AST_OP_NEG] = &IRGenerator::ir_neg;
	ast2ir_handlers[ast_operator_type::AST_OP_LT] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_LE] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_GT] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_GE] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_EQ] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_NE] = &IRGenerator::ir_compare;
	ast2ir_handlers[ast_operator_type::AST_OP_LAND] = &IRGenerator::ir_logic_and;
	ast2ir_handlers[ast_operator_type::AST_OP_LOR] = &IRGenerator::ir_logic_or;
	ast2ir_handlers[ast_operator_type::AST_OP_LNOT] = &IRGenerator::ir_logic_not;

	/* 语句 */
	ast2ir_handlers[ast_operator_type::AST_OP_ASSIGN] = &IRGenerator::ir_assign;
	ast2ir_handlers[ast_operator_type::AST_OP_RETURN] = &IRGenerator::ir_return;
	ast2ir_handlers[ast_operator_type::AST_OP_IF] = &IRGenerator::ir_if;
	ast2ir_handlers[ast_operator_type::AST_OP_WHILE] = &IRGenerator::ir_while;
	ast2ir_handlers[ast_operator_type::AST_OP_BREAK] = &IRGenerator::ir_break;
	ast2ir_handlers[ast_operator_type::AST_OP_CONTINUE] = &IRGenerator::ir_continue;

	/* 函数调用 */
	ast2ir_handlers[ast_operator_type::AST_OP_FUNC_CALL] = &IRGenerator::ir_function_call;

	/* 函数定义 */
	ast2ir_handlers[ast_operator_type::AST_OP_FUNC_DEF] = &IRGenerator::ir_function_define;
	ast2ir_handlers[ast_operator_type::AST_OP_FUNC_FORMAL_PARAMS] = &IRGenerator::ir_function_formal_params;

	/* 变量定义语句 */
	ast2ir_handlers[ast_operator_type::AST_OP_DECL_STMT] = &IRGenerator::ir_declare_statment;
	ast2ir_handlers[ast_operator_type::AST_OP_VAR_DECL] = &IRGenerator::ir_variable_declare;

	/* 语句块 */
	ast2ir_handlers[ast_operator_type::AST_OP_BLOCK] = &IRGenerator::ir_block;

	/* 编译单元 */
	ast2ir_handlers[ast_operator_type::AST_OP_COMPILE_UNIT] = &IRGenerator::ir_compile_unit;
}

/// @brief 遍历抽象语法树产生线性IR，保存到IRCode中
/// @param root 抽象语法树
/// @param IRCode 线性IR
/// @return true: 成功 false: 失败
bool IRGenerator::run()
{
	ast_node * node;

	// 从根节点进行遍历
	node = ir_visit_ast_node(root);

	return node != nullptr;
}

// bool IRGenerator::run()
// {
// 	std::cout << "[IR] start generating IR..." << std::endl;

// 	ast_node * node = ir_visit_ast_node(root);

// 	if (!node) {
// 		std::cerr << "[IR FATAL] IR generation failed at root traversal" << std::endl;
// 		return false;
// 	}

// 	std::cout << "[IR] success" << std::endl;
// 	return true;
// }

/// @brief 根据AST的节点运算符查找对应的翻译函数并执行翻译动作
/// @param node AST节点
/// @return 成功返回node节点，否则返回nullptr
ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
{
	// 空节点
	if (nullptr == node) {
		return nullptr;
	}

	bool result;

	std::unordered_map<ast_operator_type, ast2ir_handler_t>::const_iterator pIter;
	pIter = ast2ir_handlers.find(node->node_type);
	if (pIter == ast2ir_handlers.end()) {
		// 没有找到，则说明当前不支持
		result = (this->ir_default)(node);
	} else {
		result = (this->*(pIter->second))(node);
	}

	if (!result) {
		// 语义解析错误，则出错返回
		node = nullptr;
	}

	return node;
}

// ast_node * IRGenerator::ir_visit_ast_node(ast_node * node)
// {
// 	if (nullptr == node) {
// 		return nullptr;
// 	}

// 	bool result;

// 	auto it = ast2ir_handlers.find(node->node_type);

// 	if (it == ast2ir_handlers.end()) {

// 		std::cerr << "[IR ERROR] unsupported AST node type = " << (int) node->node_type << " line = " << node->line_no
// 				  << std::endl;

// 		result = this->ir_default(node);
// 	} else {
// 		result = (this->*(it->second))(node);
// 	}

// 	if (!result) {

// 		std::cerr << "[IR FAILED] node type = " << (int) node->node_type << " line = " << node->line_no
// 				  << " name = " << node->name << std::endl;

// 		return nullptr;
// 	}

// 	return node;
// }

/// @brief 未知节点类型的节点处理
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_default(ast_node * node)
{
	// 未知的节点
	printf("Unkown node(%d)\n", (int) node->node_type);
	return true;
}

/// @brief 编译单元AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compile_unit(ast_node * node)
{
	module->setCurrentFunction(nullptr);

	for (auto son: node->sons) {
		if (son && son->node_type == ast_operator_type::AST_OP_FUNC_DEF) {
			if (!registerFunctionSignature(module, son)) {
				return false;
			}
		}
	}

	for (auto son: node->sons) {

		// 遍历编译单元，要么是函数定义，要么是语句
		ast_node * son_node = ir_visit_ast_node(son);
		if (!son_node) {
			// TODO 自行追加语义错误处理
			return false;
		}
	}

	return true;
}

/// @brief 函数定义AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_define(ast_node * node)
{
	bool result;

	// 创建一个函数，用于当前函数处理
	if (module->getCurrentFunction()) {
		// 函数中嵌套定义函数，这是不允许的，错误退出
		// TODO 自行追加语义错误处理
		return false;
	}

	// 函数定义的AST包含四个孩子
	// 第一个孩子：函数返回类型
	// 第二个孩子：函数名字
	// 第三个孩子：形参列表
	// 第四个孩子：函数体即block
	ast_node * type_node = node->sons[0];
	ast_node * name_node = node->sons[1];
	ast_node * param_node = node->sons[2];
	ast_node * block_node = node->sons[3];

	// 先在编译单元第一遍中预注册函数签名，第二遍真正翻译函数体时直接复用已有函数。
	Function * newFunc = module->findFunction(name_node->name);
	if (!newFunc) {
		newFunc = registerFunctionSignature(module, node);
		if (!newFunc) {
			// 新定义的函数已经存在，则失败返回。
			// TODO 自行追加语义错误处理
			return false;
		}
	}

	// 当前函数设置有效，变更为当前的函数
	module->setCurrentFunction(newFunc);

	// 进入函数的作用域
	module->enterScope();

	// 获取函数的IR代码列表，用于后面追加指令用，注意这里用的是引用传值
	InterCode & irCode = newFunc->getInterCode();

	// 这里也可增加一个函数入口Label指令，便于后续基本块划分

	// 参考IR在入口前会先放一个label，再放entry指令
	irCode.addInst(new LabelInstruction(newFunc));
	irCode.addInst(new EntryInstruction(newFunc));

	// int类型函数默认初始化返回值为0，避免无显式return时返回值未定义
	LocalVariable * retValue = nullptr;
	if (!type_node->type->isVoidType()) {
		retValue = static_cast<LocalVariable *>(module->newVarValue(type_node->type));
		newFunc->setReturnValue(retValue);
		irCode.addInst(new MoveInstruction(newFunc, retValue, module->newConstInt(0)));
	} else {
		newFunc->setReturnValue(nullptr);
	}

	// 创建出口指令并不加入出口指令，等函数内的指令处理完毕后加入出口指令
	LabelInstruction * exitLabelInst = new LabelInstruction(newFunc);

	// 函数出口指令保存到函数信息中，因为在语义分析函数体时return语句需要跳转到函数尾部，需要这个label指令
	newFunc->setExitLabel(exitLabelInst);

	// 遍历形参，没有IR指令，不需要追加
	result = ir_function_formal_params(param_node);
	if (!result) {
		// 形参解析失败
		// TODO 自行追加语义错误处理
		return false;
	}
	node->blockInsts.addInst(param_node->blockInsts);

	// 函数内已经进入作用域，内部不再需要做变量的作用域管理
	block_node->needScope = false;

	// 遍历block
	result = ir_block(block_node);
	if (!result) {
		// block解析失败
		// TODO 自行追加语义错误处理
		return false;
	}

	// IR指令追加到当前的节点中
	node->blockInsts.addInst(block_node->blockInsts);

	// 此时，所有指令都加入到当前函数中，也就是node->blockInsts

	// node节点的指令移动到函数的IR指令列表中
	irCode.addInst(node->blockInsts);

	// 添加函数出口Label指令，主要用于return语句跳转到这里进行函数的退出
	irCode.addInst(exitLabelInst);

	// 函数出口指令
	irCode.addInst(new ExitInstruction(newFunc, retValue));

	// 恢复成外部函数
	module->setCurrentFunction(nullptr);

	// 退出函数的作用域
	module->leaveScope();

	return true;
}

/// @brief 形式参数AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_formal_params(ast_node * node)
{
	auto currentFunc = module->getCurrentFunction();
	if (!currentFunc) {
		return false;
	}

	auto & formalParams = currentFunc->getParams();

	if (!formalParams.empty() && formalParams.size() != node->sons.size()) {
		return false;
	}

	for (size_t idx = 0; idx < node->sons.size(); ++idx) {
		auto paramNode = node->sons[idx];
		if (!paramNode) {
			continue;
		}

		Type * paramType = paramNode->type;
		if (!paramType) {
			paramType = IntegerType::getTypeInt();
		}

		std::string paramName = paramNode->name;

		auto * localVar = static_cast<LocalVariable *>(module->newVarValue(paramType, paramName));
		if (!localVar) {
			return false;
		}

		FormalParam * formalParam = nullptr;
		if (!formalParams.empty()) {
			formalParam = formalParams[idx];
		} else {
			formalParam = new FormalParam(paramType, paramName);
			formalParams.push_back(formalParam);
		}

		if (!formalParam) {
			return false;
		}

		node->blockInsts.addInst(new MoveInstruction(currentFunc, localVar, formalParam));
	}

	return true;
}

/// @brief 语句块（含函数体）AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_block(ast_node * node)
{
	// 进入作用域
	if (node->needScope) {
		module->enterScope();
	}

	std::vector<ast_node *>::iterator pIter;
	for (pIter = node->sons.begin(); pIter != node->sons.end(); ++pIter) {

		// 遍历Block的每个语句，进行显示或者运算
		ast_node * temp = ir_visit_ast_node(*pIter);
		if (!temp) {
			return false;
		}

		node->blockInsts.addInst(temp->blockInsts);
	}

	// 离开作用域
	if (node->needScope) {
		module->leaveScope();
	}

	return true;
}

/// @brief return节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_return(ast_node * node)
{
	ast_node * right = nullptr;

	// return语句可能没有没有表达式，也可能有，因此这里必须进行区分判断
	if (!node->sons.empty()) {

		ast_node * son_node = node->sons[0];

		// 返回的表达式的指令保存在right节点中
		right = ir_visit_ast_node(son_node);
		if (!right) {

			// 某个变量没有定值
			return false;
		}
	}

	// 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理
	Function * currentFunc = module->getCurrentFunction();

	// 返回值存在时则移动指令到node中
	if (right) {

		// 创建临时变量保存IR的值，以及线性IR指令
		node->blockInsts.addInst(right->blockInsts);

		// 返回值赋值到函数返回值变量上，然后跳转到函数的尾部
		node->blockInsts.addInst(new MoveInstruction(currentFunc, currentFunc->getReturnValue(), right->val));

		node->val = right->val;
	} else {
		// 没有返回值
		node->val = nullptr;
	}

	// 跳转到函数的尾部出口指令上
	node->blockInsts.addInst(new GotoInstruction(currentFunc, currentFunc->getExitLabel()));

	return true;
}

/// @brief 类型叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_type(ast_node * node)
{
	// 不需要做什么，直接从节点中获取即可。

	return true;
}

/// @brief 无符号整数字面量叶子节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_uint(ast_node * node)
{
	ConstInt * val;

	// 新建一个整数常量Value
	val = module->newConstInt((int32_t) node->integer_val);

	node->val = val;

	return true;
}

/// @brief 标识符叶子节点翻译成线性中间IR，变量声明的不走这个语句
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_leaf_node_var_id(ast_node * node)
{
	Value * val;

	// 查找ID型Value
	// 变量，则需要在符号表中查找对应的值

	val = module->findVarValue(node->name);

	node->val = val;

	return true;
}

/// @brief 整数加法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_add(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
	ast_node * src2_node = node->sons[1];

	// 加法节点，左结合，先计算左节点，后计算右节点

	// 加法的左边操作数
	ast_node * left = ir_visit_ast_node(src1_node);
	if (!left) {
		// 某个变量没有定值
		return false;
	}

	// 加法的右边操作数
	ast_node * right = ir_visit_ast_node(src2_node);
	if (!right) {
		// 某个变量没有定值
		return false;
	}

	// 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

	BinaryInstruction * addInst = new BinaryInstruction(
		module->getCurrentFunction(),
		IRInstOperator::IRINST_OP_ADD_I,
		left->val,
		right->val,
		IntegerType::getTypeInt());

	// 创建临时变量保存IR的值，以及线性IR指令
	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(addInst);

	node->val = addInst;

	return true;
}

/// @brief 整数减法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_sub(ast_node * node)
{
	ast_node * src1_node = node->sons[0];
	ast_node * src2_node = node->sons[1];

	// 加法节点，左结合，先计算左节点，后计算右节点

	// 加法的左边操作数
	ast_node * left = ir_visit_ast_node(src1_node);
	if (!left) {
		// 某个变量没有定值
		return false;
	}

	// 加法的右边操作数
	ast_node * right = ir_visit_ast_node(src2_node);
	if (!right) {
		// 某个变量没有定值
		return false;
	}

	// 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

	BinaryInstruction * subInst = new BinaryInstruction(
		module->getCurrentFunction(),
		IRInstOperator::IRINST_OP_SUB_I,
		left->val,
		right->val,
		IntegerType::getTypeInt());

	// 创建临时变量保存IR的值，以及线性IR指令
	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(subInst);

	node->val = subInst;

	return true;
}

/// @brief 整数乘法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mul(ast_node * node)
{
	auto left = ir_visit_ast_node(node->sons[0]);
	auto right = ir_visit_ast_node(node->sons[1]);

	if (!left || !right)
		return false;
	auto inst = new BinaryInstruction(
		module->getCurrentFunction(),
		IRInstOperator::IRINST_OP_MUL_I,
		left->val,
		right->val,
		IntegerType::getTypeInt());

	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(inst);

	node->val = inst;
	return true;
}

/// @brief 整数除法AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_div(ast_node * node)
{
	auto left = ir_visit_ast_node(node->sons[0]);
	auto right = ir_visit_ast_node(node->sons[1]);

	if (!left || !right)
		return false;

	auto inst = new BinaryInstruction(
		module->getCurrentFunction(),
		IRInstOperator::IRINST_OP_DIV_I,
		left->val,
		right->val,
		IntegerType::getTypeInt());

	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(inst);

	node->val = inst;
	return true;
}

/// @brief 整数模运算AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_mod(ast_node * node)
{
	auto left = ir_visit_ast_node(node->sons[0]);
	auto right = ir_visit_ast_node(node->sons[1]);

	if (!left || !right)
		return false;

	auto inst = new BinaryInstruction(
		module->getCurrentFunction(),
		IRInstOperator::IRINST_OP_MOD_I,
		left->val,
		right->val,
		IntegerType::getTypeInt());

	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(inst);

	node->val = inst;

	return true;
}

/// @brief 整数取负AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_neg(ast_node * node)
{
	auto operand = ir_visit_ast_node(node->sons[0]);
	if (!operand)
		return false;

	// 常量 0
	auto zero = module->newConstInt(0);

	// 生成 0 - operand
	auto inst = new BinaryInstruction(
		module->getCurrentFunction(), IRInstOperator::IRINST_OP_SUB_I, zero, operand->val, IntegerType::getTypeInt());

	node->blockInsts.addInst(operand->blockInsts);
	node->blockInsts.addInst(inst);

	node->val = inst;

	return true;
}

/// @brief 创建当前函数中的新标签
/// @return LabelInstruction *
LabelInstruction * IRGenerator::newLabel()
{
	Function * currentFunc = module->getCurrentFunction();
	if (!currentFunc) {
		return nullptr;
	}

	return new LabelInstruction(currentFunc);
}

/// @brief 关系运算AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_compare(ast_node * node)
{
	auto left = ir_visit_ast_node(node->sons[0]);
	auto right = ir_visit_ast_node(node->sons[1]);

	if (!left || !right)
		return false;

	IRInstOperator op = IRInstOperator::IRINST_OP_CMP_EQ;
	switch (node->node_type) {
		case ast_operator_type::AST_OP_LT:
			op = IRInstOperator::IRINST_OP_CMP_LT;
			break;
		case ast_operator_type::AST_OP_LE:
			op = IRInstOperator::IRINST_OP_CMP_LE;
			break;
		case ast_operator_type::AST_OP_GT:
			op = IRInstOperator::IRINST_OP_CMP_GT;
			break;
		case ast_operator_type::AST_OP_GE:
			op = IRInstOperator::IRINST_OP_CMP_GE;
			break;
		case ast_operator_type::AST_OP_NE:
			op = IRInstOperator::IRINST_OP_CMP_NE;
			break;
		case ast_operator_type::AST_OP_EQ:
		default:
			op = IRInstOperator::IRINST_OP_CMP_EQ;
			break;
	}

	auto inst =
		new BinaryInstruction(module->getCurrentFunction(), op, left->val, right->val, IntegerType::getTypeBool());

	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(inst);
	node->val = inst;
	node->type = IntegerType::getTypeBool();

	return true;
}

/// @brief 根据条件表达式生成跳转到真假标签的IR
/// @param node 条件AST节点
/// @param trueLabel 真分支标签
/// @param falseLabel 假分支标签
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_condition(ast_node * node, LabelInstruction * trueLabel, LabelInstruction * falseLabel)
{
	if (!node || !trueLabel || !falseLabel) {
		return false;
	}

	switch (node->node_type) {
		case ast_operator_type::AST_OP_LAND: {
			auto midLabel = newLabel();
			if (!midLabel) {
				return false;
			}

			auto left = node->sons[0];
			auto right = node->sons[1];

			if (!ir_condition(left, midLabel, falseLabel)) {
				return false;
			}
			node->blockInsts.addInst(left->blockInsts);
			node->blockInsts.addInst(midLabel);
			if (!ir_condition(right, trueLabel, falseLabel)) {
				return false;
			}
			node->blockInsts.addInst(right->blockInsts);
			return true;
		}
		case ast_operator_type::AST_OP_LOR: {
			auto midLabel = newLabel();
			if (!midLabel) {
				return false;
			}

			auto left = node->sons[0];
			auto right = node->sons[1];

			if (!ir_condition(left, trueLabel, midLabel)) {
				return false;
			}
			node->blockInsts.addInst(left->blockInsts);
			node->blockInsts.addInst(midLabel);
			if (!ir_condition(right, trueLabel, falseLabel)) {
				return false;
			}
			node->blockInsts.addInst(right->blockInsts);
			return true;
		}
		case ast_operator_type::AST_OP_LNOT:
			if (!ir_condition(node->sons[0], falseLabel, trueLabel)) {
				return false;
			}
			node->blockInsts.addInst(node->sons[0]->blockInsts);
			return true;
		case ast_operator_type::AST_OP_NEG:
			if (!ir_condition(node->sons[0], trueLabel, falseLabel)) {
				return false;
			}
			node->blockInsts.addInst(node->sons[0]->blockInsts);
			return true;
		case ast_operator_type::AST_OP_LT:
		case ast_operator_type::AST_OP_LE:
		case ast_operator_type::AST_OP_GT:
		case ast_operator_type::AST_OP_GE:
		case ast_operator_type::AST_OP_EQ:
		case ast_operator_type::AST_OP_NE: {
			if (!ir_compare(node)) {
				return false;
			}
			node->blockInsts.addInst(
				new CondBranchInstruction(module->getCurrentFunction(), node->val, trueLabel, falseLabel));
			return true;
		}
		default: {
			auto valueNode = ir_visit_ast_node(node);
			if (!valueNode) {
				return false;
			}
			if (!valueNode->val) {
				return false;
			}
			auto zero = module->newConstInt(0);
			auto condInst = new BinaryInstruction(
				module->getCurrentFunction(),
				IRInstOperator::IRINST_OP_CMP_NE,
				valueNode->val,
				zero,
				IntegerType::getTypeBool());
			node->blockInsts.addInst(condInst);
			node->blockInsts.addInst(
				new CondBranchInstruction(module->getCurrentFunction(), condInst, trueLabel, falseLabel));
			node->val = valueNode->val;
			return true;
		}
	}
}

/// @brief 将条件表达式物化为0/1整数值
/// @param node 条件AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_condition_value(ast_node * node)
{
	auto currentFunc = module->getCurrentFunction();
	auto trueLabel = newLabel();
	auto falseLabel = newLabel();
	auto exitLabel = newLabel();

	if (!currentFunc || !trueLabel || !falseLabel || !exitLabel) {
		return false;
	}

	if (!ir_condition(node, trueLabel, falseLabel)) {
		return false;
	}

	auto resultVar = module->newVarValue(IntegerType::getTypeInt());
	if (!resultVar) {
		return false;
	}

	node->blockInsts.addInst(trueLabel);
	node->blockInsts.addInst(new MoveInstruction(currentFunc, resultVar, module->newConstInt(1)));
	node->blockInsts.addInst(new GotoInstruction(currentFunc, exitLabel));
	node->blockInsts.addInst(falseLabel);
	node->blockInsts.addInst(new MoveInstruction(currentFunc, resultVar, module->newConstInt(0)));
	node->blockInsts.addInst(new GotoInstruction(currentFunc, exitLabel));
	node->blockInsts.addInst(exitLabel);

	node->val = resultVar;
	node->type = IntegerType::getTypeInt();

	return true;
}

/// @brief 逻辑与AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logic_and(ast_node * node)
{
	return ir_condition_value(node);
}

/// @brief 逻辑或AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logic_or(ast_node * node)
{
	return ir_condition_value(node);
}

/// @brief 逻辑非AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_logic_not(ast_node * node)
{
	return ir_condition_value(node);
}

/// @brief if语句AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_if(ast_node * node)
{
	auto currentFunc = module->getCurrentFunction();
	auto trueLabel = newLabel();
	auto falseLabel = newLabel();
	auto exitLabel = newLabel();

	if (!currentFunc || !trueLabel || !falseLabel || !exitLabel) {
		return false;
	}

	if (!ir_condition(node->sons[0], trueLabel, falseLabel)) {
		return false;
	}

	auto thenNode = node->sons[1];
	ast_node * elseNode = (node->sons.size() > 2) ? node->sons[2] : nullptr;

	node->blockInsts.addInst(node->sons[0]->blockInsts);
	node->blockInsts.addInst(trueLabel);
	if (!ir_visit_ast_node(thenNode)) {
		return false;
	}
	node->blockInsts.addInst(thenNode->blockInsts);
	node->blockInsts.addInst(new GotoInstruction(currentFunc, exitLabel));
	node->blockInsts.addInst(falseLabel);
	if (elseNode) {
		if (!ir_visit_ast_node(elseNode)) {
			return false;
		}
		node->blockInsts.addInst(elseNode->blockInsts);
	}
	node->blockInsts.addInst(exitLabel);

	return true;
}

/// @brief while语句AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_while(ast_node * node)
{
	auto currentFunc = module->getCurrentFunction();
	auto entryLabel = newLabel();
	auto bodyLabel = newLabel();
	auto exitLabel = newLabel();

	if (!currentFunc || !entryLabel || !bodyLabel || !exitLabel) {
		return false;
	}

	node->blockInsts.addInst(entryLabel);
	if (!ir_condition(node->sons[0], bodyLabel, exitLabel)) {
		return false;
	}
	node->blockInsts.addInst(node->sons[0]->blockInsts);
	node->blockInsts.addInst(bodyLabel);

	loopStack.push_back({entryLabel, exitLabel});
	auto bodyNode = node->sons[1];
	if (!ir_visit_ast_node(bodyNode)) {
		loopStack.pop_back();
		return false;
	}
	loopStack.pop_back();

	node->blockInsts.addInst(bodyNode->blockInsts);
	node->blockInsts.addInst(new GotoInstruction(currentFunc, entryLabel));
	node->blockInsts.addInst(exitLabel);

	return true;
}

/// @brief break语句AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_break(ast_node * node)
{
	if (loopStack.empty()) {
		minic_log(LOG_ERROR, "break语句不在循环中");
		return false;
	}

	auto currentFunc = module->getCurrentFunction();
	node->blockInsts.addInst(new GotoInstruction(currentFunc, loopStack.back().exitLabel));
	return true;
}

/// @brief continue语句AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_continue(ast_node * node)
{
	if (loopStack.empty()) {
		minic_log(LOG_ERROR, "continue语句不在循环中");
		return false;
	}

	auto currentFunc = module->getCurrentFunction();
	node->blockInsts.addInst(new GotoInstruction(currentFunc, loopStack.back().entryLabel));
	return true;
}

/// @brief 赋值AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_assign(ast_node * node)
{
	ast_node * son1_node = node->sons[0];
	ast_node * son2_node = node->sons[1];

	// 赋值节点，自右往左运算

	// 赋值运算符的左侧操作数
	ast_node * left = ir_visit_ast_node(son1_node);
	if (!left) {
		// 某个变量没有定值
		// 这里缺省设置变量不存在则创建，因此这里不会错误
		return false;
	}

	// 赋值运算符的右侧操作数
	ast_node * right = ir_visit_ast_node(son2_node);
	if (!right) {
		// 某个变量没有定值
		return false;
	}

	// 这里只处理整型的数据，如需支持实数，则需要针对类型进行处理

	MoveInstruction * movInst = new MoveInstruction(module->getCurrentFunction(), left->val, right->val);

	// 创建临时变量保存IR的值，以及线性IR指令
	node->blockInsts.addInst(right->blockInsts);
	node->blockInsts.addInst(left->blockInsts);
	node->blockInsts.addInst(movInst);

	// 这里假定赋值的类型是一致的
	node->val = movInst;

	return true;
}

/// @brief 变量声明语句节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_declare_statment(ast_node * node)
{
	bool result = false;

	for (auto & child: node->sons) {

		// 遍历每个变量声明
		result = ir_variable_declare(child);
		if (!result) {
			break;
		}
	}

	return result;
}

/// @brief 变量定声明节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_variable_declare(ast_node * node)
{
	// 共有两个孩子，第一个类型，第二个变量名

	// TODO 这里可强化类型等检查

	node->val = module->newVarValue(node->sons[0]->type, node->sons[1]->name);

	return true;
}

/// @brief 函数调用AST节点翻译成线性中间IR
/// @param node AST节点
/// @return 翻译是否成功，true：成功，false：失败
bool IRGenerator::ir_function_call(ast_node * node)
{
	std::vector<Value *> realParams;

	// 获取当前正在处理的函数
	Function * currentFunc = module->getCurrentFunction();

	// 函数调用的节点包含两个节点：
	// 第一个节点：函数名节点
	// 第二个节点：实参列表节点

	std::string funcName = node->sons[0]->name;
	int64_t lineno = node->sons[0]->line_no;

	ast_node * paramsNode = node->sons[1];

	// 根据函数名查找函数，看是否存在。若不存在则出错
	// 这里约定函数必须先定义后使用
	auto calledFunction = module->findFunction(funcName);
	if (nullptr == calledFunction) {
		minic_log(LOG_ERROR, "函数(%s)未定义或声明", funcName.c_str());
		return false;
	}

	// 当前函数存在函数调用
	currentFunc->setExistFuncCall(true);

	// 如果没有孩子，也认为是没有参数
	if (!paramsNode->sons.empty()) {

		int32_t argsCount = (int32_t) paramsNode->sons.size();

		// 当前函数中调用函数实参个数最大值统计，实际上是统计实参传参需在栈中分配的大小
		// 因为目前的语言支持的int和float都是四字节的，只统计个数即可
		if (argsCount > currentFunc->getMaxFuncCallArgCnt()) {
			currentFunc->setMaxFuncCallArgCnt(argsCount);
		}

		// 遍历参数列表，孩子是表达式
		// 这里自左往右计算表达式
		for (auto son: paramsNode->sons) {

			// 遍历Block的每个语句，进行显示或者运算
			ast_node * temp = ir_visit_ast_node(son);
			if (!temp) {
				return false;
			}

			realParams.push_back(temp->val);
			node->blockInsts.addInst(temp->blockInsts);
		}
	}

	// TODO 这里请追加函数调用的语义错误检查，这里只进行了函数参数的个数检查等，其它请自行追加。
	if (realParams.size() != calledFunction->getParams().size()) {
		// 函数参数的个数不一致，语义错误
		minic_log(LOG_ERROR, "第%lld行的被调用函数(%s)未定义或声明", (long long) lineno, funcName.c_str());
		return false;
	}

	// 返回调用有返回值，则需要分配临时变量，用于保存函数调用的返回值
	Type * type = calledFunction->getReturnType();

	FuncCallInstruction * funcCallInst = new FuncCallInstruction(currentFunc, calledFunction, realParams, type);

	// 创建函数调用指令
	node->blockInsts.addInst(funcCallInst);

	// 函数调用结果Value保存到node中，可能为空，上层节点可利用这个值
	node->val = funcCallInst;

	return true;
}

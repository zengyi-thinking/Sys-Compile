/**
 * @file optimizer.h
 * @brief 代码优化器(Optimizer)的头文件
 *
 * 本模块负责对中间代码进行优化，提高生成的目标代码效率。
 *
 * ==================== 代码优化的作用 ====================
 * 代码优化是编译过程的重要阶段，旨在提高代码的执行效率，
 * 同时不改变程序的原有语义。
 *
 * ==================== 常见优化技术 ====================
 * 1. 常量折叠(Constant Folding): 编译时计算常量表达式
 *    例: x = 3 + 5  →  x = 8
 *
 * 2. 死代码消除(Dead Code Elimination): 删除不会执行的代码
 *    例: if (false) { ... }  →  (删除)
 *
 * 3. 常量传播(Constant Propagation): 用常量值替换变量
 *    例: x = 5; y = x + 3  →  x = 5; y = 5 + 3
 *
 * 4. 控制流简化: 简化跳转指令
 */

#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../codegen/code_generator.h"
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>

/**
 * @class Optimizer
 * @brief 代码优化器类
 *
 * 对中间代码进行多种优化，包括常量折叠、死代码消除等。
 */
class Optimizer {
public:
    /**
     * @brief 构造函数
     */
    Optimizer();

    /**
     * @brief 析构函数
     */
    ~Optimizer();

    /**
     * @brief 对所有函数进行优化
     * @param functions 函数映射表
     */
    void optimize(std::map<std::string, std::shared_ptr<Function>>& functions);

    /**
     * @brief 获取常量折叠次数
     * @return 常量折叠次数
     */
    int getConstantFoldings() const { return constantFoldings; }

    /**
     * @brief 获取死代码消除次数
     * @return 死代码消除次数
     */
    int getDeadCodeEliminations() const { return deadCodeEliminations; }

private:
    int constantFoldings;         ///< 常量折叠次数统计
    int deadCodeEliminations;     ///< 死代码消除次数统计

    // ============ 优化函数 ============

    /**
     * @brief 优化单个函数
     * @param func 要优化的函数
     */
    void optimizeFunction(std::shared_ptr<Function> func);

    /**
     * @brief 优化基本块
     * @param block 要优化的基本块
     */
    void optimizeBlock(std::shared_ptr<BasicBlock> block);

    /**
     * @brief 常量折叠优化
     * @param block 要优化的基本块
     * @return 是否进行了优化
     *
     * 在编译时计算常量表达式，如: t1 = 3 + 5 → t1 = 8
     */
    bool constantFoldBlock(std::shared_ptr<BasicBlock> block);

    /**
     * @brief 死代码消除
     * @param block 要优化的基本块
     * @return 是否进行了优化
     *
     * 删除不会被执行的代码或对结果无影响的代码。
     */
    bool eliminateDeadCode(std::shared_ptr<BasicBlock> block);

    /**
     * @brief 控制流简化
     * @param func 要优化的函数
     * @return 是否进行了优化
     *
     * 简化跳转指令，如删除不必要的跳转。
     */
    bool simplifyControlFlow(std::shared_ptr<Function> func);

    // ============ 辅助函数 ============

    /**
     * @brief 检查字符串是否为常量
     * @param value 要检查的值
     * @return 是常量返回true
     */
    bool isConstant(const std::string& value);

    /**
     * @brief 检查字符串是否为数值
     * @param value 要检查的值
     * @return 是数值返回true
     */
    bool isNumeric(const std::string& value);

    /**
     * @brief 将字符串转换为数值
     * @param value 要转换的字符串
     * @return 数值
     */
    double toNumber(const std::string& value);

    /**
     * @brief 将数值转换为字符串
     * @param value 数值
     * @param isInt 是否为整数
     * @return 字符串表示
     */
    std::string fromNumber(double value, bool isInt);

    /**
     * @brief 常量传播映射表
     *
     * 记录变量到其常量值的映射，用于常量传播优化。
     */
    std::map<std::string, std::string> constantPropagation;

    /**
     * @brief 活变量集合
     *
     * 记录在当前点仍然"活跃"（后续会被使用）的变量。
     */
    std::set<std::string> liveVariables;

    /**
     * @brief 计算二元运算的结果
     * @param op 操作码
     * @param left 左操作数
     * @param right 右操作数
     * @param result 输出结果
     * @return 计算成功返回true
     */
    bool evaluateBinaryOp(OpCode op, const std::string& left, const std::string& right, std::string& result);

    /**
     * @brief 计算一元运算的结果
     * @param op 操作码
     * @param operand 操作数
     * @param result 输出结果
     * @return 计算成功返回true
     */
    bool evaluateUnaryOp(OpCode op, const std::string& operand, std::string& result);
};

#endif // OPTIMIZER_H

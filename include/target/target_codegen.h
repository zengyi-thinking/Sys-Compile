/**
 * @file target_codegen.h
 * @brief 目标代码生成器(Target Code Generator)的头文件
 *
 * 本模块负责将中间代码转换为特定平台的汇编代码。
 *
 * ==================== 目标代码生成的作用 ====================
 * 目标代码生成是编译的最后阶段，将平台无关的中间代码
 * 转换为特定处理器架构的机器码或汇编代码。
 *
 * ==================== 支持的平台 ====================
 * - x86-64: Intel/AMD 64位架构 (使用Intel语法)
 * - RISC-V: 开源指令集架构 (64位)
 *
 * ==================== 编译流程回顾 ====================
 * 源代码 → 词法分析 → 语法分析 → AST → 语义分析
 *       → 中间代码 → 优化 → 目标代码生成 → 汇编代码
 */

#ifndef TARGET_CODEGEN_H
#define TARGET_CODEGEN_H

#include "../codegen/code_generator.h"
#include <string>
#include <map>
#include <memory>
#include <sstream>

/**
 * @enum TargetArch
 * @brief 目标架构枚举
 *
 * 定义支持的目标处理器架构。
 */
enum class TargetArch {
    X86_64,   ///< x86-64架构 (Intel/AMD 64位)
    RISCV64   ///< RISC-V 64位架构
};

/**
 * @class TargetCodeGenerator
 * @brief 目标代码生成器类
 *
 * 将中间代码(三地址码)转换为特定平台的汇编代码。
 */
class TargetCodeGenerator {
private:
    TargetArch arch;                              ///< 目标架构
    std::ostringstream assemblyCode;              ///< 生成的汇编代码
    std::map<std::string, int> localVarOffsets;   ///< 局部变量栈偏移
    int stackPointer;                             ///< 栈指针位置
    std::map<std::string, std::string> registerAllocation;  ///< 寄存器分配

    // ============ 寄存器管理 ============

    /**
     * @brief 分配一个寄存器
     * @return 寄存器名
     */
    std::string allocRegister();

    /**
     * @brief 释放一个寄存器
     * @param reg 要释放的寄存器
     */
    void freeRegister(const std::string& reg);

    /**
     * @brief 获取变量对应的寄存器
     * @param var 变量名
     * @return 寄存器名
     */
    std::string getRegister(const std::string& var);

    // ============ 函数框架生成 ============

    /**
     * @brief 生成函数序言
     * @param funcName 函数名
     *
     * 函数序言通常包括：
     * - 保存旧的基指针
     * - 设置新的基指针
     * - 为局部变量分配栈空间
     */
    void generatePrologue(const std::string& funcName);

    /**
     * @brief 生成函数尾声
     *
     * 函数尾声通常包括：
     * - 恢复栈空间
     * - 恢复旧的基指针
     * - 返回调用者
     */
    void generateEpilogue();

    /**
     * @brief 生成单条指令的汇编代码
     * @param inst 中间代码指令
     */
    void generateInstruction(std::shared_ptr<Instruction> inst);

    // ============ 指令转换 ============

    /**
     * @brief 将中间代码操作码转换为x86-64指令
     * @param op 中间代码操作码
     * @return x86-64指令字符串
     */
    std::string opToX86Op(OpCode op);

    /**
     * @brief 将中间代码操作码转换为RISC-V指令
     * @param op 中间代码操作码
     * @return RISC-V指令字符串
     */
    std::string opToRISCVOps(OpCode op);

    /**
     * @brief 生成x86-64指令
     * @param inst 中间代码指令
     */
    void generateX86Instruction(std::shared_ptr<Instruction> inst);

    /**
     * @brief 生成RISC-V指令
     * @param inst 中间代码指令
     */
    void generateRISCVInstruction(std::shared_ptr<Instruction> inst);

    /**
     * @brief 发出一行汇编代码
     * @param line 要发出的代码行
     */
    void emitLine(const std::string& line);

public:
    /**
     * @brief 构造函数
     * @param a 目标架构
     */
    TargetCodeGenerator(TargetArch a = TargetArch::X86_64);

    /**
     * @brief 析构函数
     */
    ~TargetCodeGenerator();

    /**
     * @brief 生成目标代码
     * @param functions 函数映射表
     */
    void generate(std::map<std::string, std::shared_ptr<Function>>& functions);

    /**
     * @brief 获取生成的汇编代码
     * @return 汇编代码字符串
     */
    std::string getAssembly() const { return assemblyCode.str(); }

    /**
     * @brief 设置目标架构
     * @param a 目标架构
     */
    void setArchitecture(TargetArch a) { arch = a; }
};

#endif // TARGET_CODEGEN_H

/**
 * @file code_generator.h
 * @brief 中间代码生成器(Code Generator)的头文件
 *
 * 本模块负责将抽象语法树(AST)转换为中间代码(三地址码)。
 *
 * ==================== 中间代码的作用 ====================
 * 中间代码是介于源语言和目标机器语言之间的一种表示形式。
 * 使用中间代码的好处：
 * 1. 与目标平台无关，便于移植
 * 2. 便于进行代码优化
 * 3. 简化了后端代码生成
 *
 * ==================== 三地址码 ====================
 * 三地址码是一种中间表示形式，每条指令最多包含三个操作数。
 *
 * 示例:
 * @code
 * 源代码:   a = b + c * d
 * 三地址码: t1 = c * d
 *           t2 = b + t1
 *           a = t2
 * @endcode
 */

#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "../ast/ast.h"
#include "semantic/semantic_analyzer.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

/**
 * @enum OpCode
 * @brief 中间代码的操作码枚举
 *
 * 定义了所有支持的三地址码指令类型。
 */
enum class OpCode {
    // ============ 算术运算 ============
    ADD,     ///< 加法: result = arg1 + arg2
    SUB,     ///< 减法: result = arg1 - arg2
    MUL,     ///< 乘法: result = arg1 * arg2
    DIV,     ///< 除法: result = arg1 / arg2
    MOD,     ///< 取模: result = arg1 % arg2
    NEG,     ///< 取负: result = -arg1

    // ============ 逻辑运算 ============
    AND,     ///< 逻辑与: result = arg1 && arg2
    OR,      ///< 逻辑或: result = arg1 || arg2
    NOT,     ///< 逻辑非: result = !arg1

    // ============ 关系运算 ============
    EQ,      ///< 等于: result = (arg1 == arg2)
    NE,      ///< 不等: result = (arg1 != arg2)
    LT,      ///< 小于: result = (arg1 < arg2)
    LE,      ///< 小于等于: result = (arg1 <= arg2)
    GT,      ///< 大于: result = (arg1 > arg2)
    GE,      ///< 大于等于: result = (arg1 >= arg2)

    // ============ 赋值与控制 ============
    ASSIGN,  ///< 赋值: result = arg1
    PARAM,   ///< 参数传递: 压栈arg1
    CALL,    ///< 函数调用: 调用arg1，结果存入result
    RETURN,  ///< 函数返回: 返回arg1

    // ============ 跳转指令 ============
    LABEL,   ///< 标签定义
    JUMP,    ///< 无条件跳转
    JZ,      ///< 条件跳转: arg1为0时跳转到label
    JNZ,     ///< 条件跳转: arg1非0时跳转到label

    // ============ 内存操作 ============
    ALLOC,   ///< 分配栈空间

    // ============ 加载与存储 ============
    LOAD,    ///< 从内存加载: result = *arg1
    STORE,   ///< 存储到内存: *result = arg1

    // ============ 类型转换 ============
    CAST_INT,    ///< 转换为int
    CAST_FLOAT   ///< 转换为float
};

/**
 * @class Instruction
 * @brief 三地址码指令类
 *
 * 表示一条三地址码指令。
 */
class Instruction {
public:
    OpCode op;              ///< 操作码
    std::string result;     ///< 结果操作数
    std::string arg1;       ///< 第一个参数
    std::string arg2;       ///< 第二个参数
    std::string label;      ///< 标签(用于跳转指令)

    /**
     * @brief 构造无参数指令
     */
    Instruction(OpCode o) : op(o) {}

    /**
     * @brief 构造单参数指令
     * @param o 操作码
     * @param r 结果操作数
     */
    Instruction(OpCode o, const std::string& r) : op(o), result(r) {}

    /**
     * @brief 构造双参数指令
     * @param o 操作码
     * @param r 结果操作数
     * @param a1 第一个参数
     */
    Instruction(OpCode o, const std::string& r, const std::string& a1)
        : op(o), result(r), arg1(a1) {}

    /**
     * @brief 构造三参数指令
     * @param o 操作码
     * @param r 结果操作数
     * @param a1 第一个参数
     * @param a2 第二个参数
     */
    Instruction(OpCode o, const std::string& r, const std::string& a1, const std::string& a2)
        : op(o), result(r), arg1(a1), arg2(a2) {}

    /**
     * @brief 构造标签指令
     * @param o 操作码
     * @param l 标签名
     * @param isLabel 占位参数，用于区分重载
     */
    Instruction(OpCode o, const std::string& l, bool isLabel)
        : op(o), label(l) {}

    /**
     * @brief 将指令转换为字符串
     * @return 指令的字符串表示
     */
    std::string toString() const;
};

/**
 * @class BasicBlock
 * @brief 基本块类
 *
 * 基本块是只有一个入口点和一个出口点的连续指令序列。
 * 用于控制流分析和优化。
 */
class BasicBlock {
public:
    std::string label;                                          ///< 基本块标签
    std::vector<std::shared_ptr<Instruction>> instructions;     ///< 指令列表
    std::shared_ptr<BasicBlock> trueBranch;                     ///< 真分支(条件跳转目标)
    std::shared_ptr<BasicBlock> falseBranch;                    ///< 假分支(条件跳转目标)
    std::shared_ptr<BasicBlock> nextBlock;                      ///< 下一个基本块

    /**
     * @brief 构造函数
     * @param l 基本块标签
     */
    BasicBlock(const std::string& l) : label(l) {}

    /**
     * @brief 添加指令到基本块
     * @param inst 要添加的指令
     */
    void addInstruction(std::shared_ptr<Instruction> inst) {
        instructions.push_back(inst);
    }

    /**
     * @brief 将基本块转换为字符串
     * @return 基本块的字符串表示
     */
    std::string toString() const;
};

/**
 * @class Function
 * @brief 函数的中间代码表示
 *
 * 包含函数的所有基本信息和控制流。
 */
class Function {
public:
    std::string name;                                   ///< 函数名
    std::shared_ptr<BasicBlock> entryBlock;            ///< 入口基本块
    std::vector<std::string> params;                   ///< 参数列表
    std::string returnType;                            ///< 返回类型
    std::map<std::string, int> localVarOffsets;        ///< 局部变量栈偏移
    int stackSize;                                     ///< 栈大小

    /**
     * @brief 构造函数
     * @param n 函数名
     */
    Function(const std::string& n) : name(n), stackSize(0) {}

    /**
     * @brief 将函数转换为字符串
     * @return 函数的字符串表示
     */
    std::string toString() const;
};

/**
 * @class CodeGenerator
 * @brief 中间代码生成器类
 *
 * 遍历AST，生成三地址码形式的中间代码。
 */
class CodeGenerator {
private:
    std::map<std::string, std::shared_ptr<Function>> functions;   ///< 所有函数
    std::shared_ptr<Function> currentFunction;                    ///< 当前函数
    std::shared_ptr<BasicBlock> currentBlock;                     ///< 当前基本块
    int tempCounter;                                              ///< 临时变量计数器
    int labelCounter;                                             ///< 标签计数器
    int stackOffset;                                              ///< 栈偏移量
    std::shared_ptr<Scope> globalScope;                           ///< 全局作用域

    /**
     * @brief 生成新的临时变量名
     * @return 临时变量名(如 t0, t1, ...)
     */
    std::string newTemp();

    /**
     * @brief 生成新的标签名
     * @return 标签名(如 L0, L1, ...)
     */
    std::string newLabel();

    /**
     * @brief 发出一条指令
     * @param inst 要发出的指令
     */
    void emit(std::shared_ptr<Instruction> inst);

    /**
     * @brief 创建新的基本块
     * @param prefix 标签前缀
     * @return 新的基本块
     */
    std::shared_ptr<BasicBlock> newBasicBlock(const std::string& prefix = "");

    /**
     * @brief 设置当前基本块
     * @param block 要设置的基本块
     */
    void setBlock(std::shared_ptr<BasicBlock> block);

    // ============ 代码生成函数 ============

    /**
     * @brief 生成表达式代码
     * @param node 表达式节点
     * @return 存放结果的临时变量名
     */
    std::string generateExpr(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成语句代码
     * @param node 语句节点
     */
    void generateStmt(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成代码块
     * @param node 代码块节点
     */
    void generateBlock(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成if语句代码
     * @param node if语句节点
     */
    void generateIf(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成while语句代码
     * @param node while语句节点
     */
    void generateWhile(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成return语句代码
     * @param node return语句节点
     */
    void generateReturn(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成函数代码
     * @param node 函数定义节点
     */
    void generateFunction(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成变量声明代码
     * @param node 声明节点
     */
    void generateDecl(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成赋值语句代码
     * @param node 赋值节点
     */
    void generateAssignment(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成函数调用代码
     * @param node 函数调用节点
     */
    void generateCall(std::shared_ptr<ASTNode> node);

    /**
     * @brief 生成数组访问代码
     * @param node 数组索引节点
     * @return 存放结果的临时变量名
     */
    std::string generateArrayAccess(std::shared_ptr<ASTNode> node);

    // ============ 辅助函数 ============

    /**
     * @brief 将操作码转换为字符串
     * @param op 操作码
     * @return 操作码字符串
     */
    std::string opToString(OpCode op);

    /**
     * @brief 获取类型前缀
     * @param kind 类型种类
     * @return 类型前缀字符串
     */
    std::string typeToPrefix(TypeKind kind);

public:
    /**
     * @brief 构造函数
     * @param scope 全局作用域
     */
    CodeGenerator(std::shared_ptr<Scope> scope);

    /**
     * @brief 析构函数
     */
    ~CodeGenerator();

    /**
     * @brief 生成中间代码
     * @param root AST根节点
     */
    void generate(std::shared_ptr<ASTNode> root);

    /**
     * @brief 获取生成的中间代码字符串
     * @return 中间代码字符串
     */
    std::string getGeneratedCode() const;

    /**
     * @brief 获取所有函数
     * @return 函数映射表
     */
    const std::map<std::string, std::shared_ptr<Function>>& getFunctions() const {
        return functions;
    }
};

#endif // CODE_GENERATOR_H

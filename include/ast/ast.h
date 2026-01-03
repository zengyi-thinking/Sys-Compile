/**
 * @file ast.h
 * @brief 抽象语法树(Abstract Syntax Tree)模块的头文件
 *
 * 本模块定义了编译器的中间表示——抽象语法树(AST)的数据结构。
 * AST 是编译器将源代码转换为机器码的重要中间步骤，用于表示程序的语法结构。
 *
 * 编译器流程:
 * 源代码 → 词法分析 → 语法分析 → AST → 语义分析 → 中间代码生成 → 优化 → 目标代码生成
 */

#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

/**
 * @enum NodeType
 * @brief AST节点的类型枚举
 *
 * 每种节点类型对应语法中的一个构造，如声明、语句、表达式等。
 */
enum NodeType {
    // ============ 程序结构 ============
    NODE_COMP_UNIT,    ///< 编译单元(Compilation Unit) - 整个程序的根节点

    // ============ 声明 ============
    NODE_DECL,         ///< 变量声明(Declaration) - 包含类型和变量名
    NODE_CONST_DECL,   ///< 常量声明(Const Declaration) - const变量
    NODE_FUNC_DEF,     ///< 函数定义(Function Definition)
    NODE_TYPE,         ///< 类型节点 - 存储类型信息(int/float/void)

    // ============ 语句 ============
    NODE_BLOCK,        ///< 代码块(Block) - 用花括号括起的语句序列
    NODE_STMT,         ///< 普通语句(Statement)
    NODE_IF,           ///< if条件语句
    NODE_WHILE,        ///< while循环语句
    NODE_RETURN,       ///< return返回语句
    NODE_BREAK,        ///< break跳出循环语句
    NODE_CONTINUE,     ///< continue继续循环语句

    // ============ 表达式 ============
    NODE_EXP,          ///< 通用表达式(Expression)
    NODE_ASSIGN,       ///< 赋值表达式(=)
    NODE_BINARY_OP,    ///< 二元运算符(+, -, *, /, %, &&, ||, <, >, ==, etc.)
    NODE_UNARY_OP,     ///< 一元运算符(+, -, !, 类型转换)
    NODE_CALL,         ///< 函数调用表达式

    // ============ 基础元素 ============
    NODE_IDENTIFIER,   ///< 标识符(变量名、函数名等)
    NODE_INT_CONST,    ///< 整型常量
    NODE_FLOAT_CONST,  ///< 浮点型常量

    // ============ 数组 ============
    NODE_ARRAY,        ///< 数组声明
    NODE_INDEX,        ///< 数组索引操作(如 a[i])
};

/**
 * @enum TypeKind
 * @brief 语义类型的种类枚举
 *
 * 用于语义分析阶段的类型检查。
 */
enum TypeKind {
    TYPE_INT,    ///< 整型
    TYPE_FLOAT,  ///< 浮点型
    TYPE_VOID,   ///< 无类型(用于函数无返回值)
    TYPE_ARRAY   ///< 数组类型
};

/**
 * @class Type
 * @brief 类型信息类
 *
 * 存储类型相关的信息，包括类型种类、数组基类型和数组大小。
 * 主要用于语义分析中的类型检查。
 */
class Type {
public:
    TypeKind kind;                      ///< 类型种类
    std::shared_ptr<Type> base_type;    ///< 数组的基类型(如 int[] 的 int)
    int array_size;                     ///< 数组大小

    /**
     * @brief 构造基础类型
     * @param k 类型种类
     */
    Type(TypeKind k) : kind(k), array_size(0) {}

    /**
     * @brief 构造数组类型
     * @param k 类型种类(应为 TYPE_ARRAY)
     * @param base 数组的基类型
     * @param size 数组大小
     */
    Type(TypeKind k, std::shared_ptr<Type> base, int size)
        : kind(k), base_type(base), array_size(size) {}
};

/**
 * @class ASTNode
 * @brief 抽象语法树节点类
 *
 * 每个节点代表源代码中的一个语法构造。
 * 节点包含类型、子节点列表、可选的值(如标识符名、运算符)和行号。
 *
 * 节点使用智能指针(std::shared_ptr)管理，确保内存安全。
 */
class ASTNode {
public:
    NodeType type;                                   ///< 节点类型
    std::vector<std::shared_ptr<ASTNode>> children;  ///< 子节点列表
    std::string value;                               ///< 节点值(如标识符名、运算符、常量值等)
    int line_number;                                 ///< 源代码中的行号(用于错误报告)

    /**
     * @brief 构造一个无值的AST节点
     * @param t 节点类型
     */
    ASTNode(NodeType t) : type(t), line_number(0) {}

    /**
     * @brief 构造一个带值的AST节点
     * @param t 节点类型
     * @param v 节点值
     */
    ASTNode(NodeType t, const std::string& v) : type(t), value(v), line_number(0) {}

    /**
     * @brief 添加子节点
     * @param child 要添加的子节点(shared_ptr版本)
     */
    void addChild(std::shared_ptr<ASTNode> child) {
        children.push_back(child);
    }

    /**
     * @brief 添加子节点(原始指针重载)
     * @param child 要添加的子节点(原始指针)
     *
     * 此重载主要用于parser.y中的 Bison 语法规则。
     * 使用空 deleter，因为节点的生命周期由 parser 中的 node_registry 管理。
     */
    void addChild(ASTNode* child) {
        children.push_back(std::shared_ptr<ASTNode>(child, [](ASTNode*){}));
    }

    /**
     * @brief 打印AST树(用于调试)
     * @param indent 缩进空格数
     */
    void print(int indent = 0);
};

/**
 * @brief 将节点类型枚举转换为可读字符串
 * @param type 节点类型
 * @return 类型名称字符串
 */
std::string nodeTypeToString(NodeType type);

#endif // AST_H

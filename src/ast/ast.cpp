/**
 * @file ast.cpp
 * @brief 抽象语法树(AST)模块的实现文件
 *
 * 实现AST节点的打印功能和类型转换功能。
 */

#include "ast/ast.h"
#include <iostream>

/**
 * @brief 将节点类型枚举转换为可读字符串
 * @param type 节点类型枚举值
 * @return 对应的类型名称字符串
 *
 * 此函数主要用于调试输出，将内部枚举值转换为人类可读的名称。
 */
std::string nodeTypeToString(NodeType type) {
    switch (type) {
        case NODE_COMP_UNIT: return "CompUnit";
        case NODE_DECL: return "Decl";
        case NODE_FUNC_DEF: return "FuncDef";
        case NODE_BLOCK: return "Block";
        case NODE_STMT: return "Stmt";
        case NODE_IF: return "If";
        case NODE_WHILE: return "While";
        case NODE_RETURN: return "Return";
        case NODE_BREAK: return "Break";
        case NODE_CONTINUE: return "Continue";
        case NODE_EXP: return "Exp";
        case NODE_ASSIGN: return "Assign";
        case NODE_BINARY_OP: return "BinaryOp";
        case NODE_UNARY_OP: return "UnaryOp";
        case NODE_CALL: return "Call";
        case NODE_IDENTIFIER: return "Identifier";
        case NODE_INT_CONST: return "IntConst";
        case NODE_FLOAT_CONST: return "FloatConst";
        case NODE_ARRAY: return "Array";
        case NODE_INDEX: return "Index";
        case NODE_TYPE: return "Type";
        default: return "Unknown";
    }
}

/**
 * @brief 打印AST树(用于调试)
 * @param indent 缩进空格数，用于层次化显示
 *
 * 以树状结构递归打印AST节点及其所有子节点。
 * 输出格式: [缩进][节点类型]: [值] (行号)
 *
 * 示例输出:
 * @code
 * CompUnit
 *   FuncDef: main
 *     Type: void
 *     Block
 *       Decl: int
 *         Identifier: a
 * @endcode
 */
void ASTNode::print(int indent) {
    // 生成缩进字符串(每层2个空格)
    std::string indent_str(indent * 2, ' ');

    // 打印节点类型
    std::cout << indent_str << nodeTypeToString(type);

    // 打印节点值(如果有且长度合理)
    if (!value.empty() && value.length() < 100) {
        std::cout << ": " << value;
    }

    // 打印行号(如果有)
    if (line_number > 0) {
        std::cout << " (line " << line_number << ")";
    }

    // 换行
    std::cout << std::endl;

    // 递归打印所有子节点
    for (auto& child : children) {
        if (!child) continue;
        if (child) {
            child->print(indent + 1);
        }
    }
}

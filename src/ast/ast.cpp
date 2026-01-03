#include "ast/ast.h"
#include <iostream>

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

void ASTNode::print(int indent) {
    std::string indent_str(indent * 2, ' ');
    std::cout << indent_str << nodeTypeToString(type);
    
    if (!value.empty() && value.length() < 100) {
        std::cout << ": " << value;
    }
    
    if (line_number > 0) {
        std::cout << " (line " << line_number << ")";
    }
    
    std::cout << std::endl;
    
    for (auto& child : children) {
        if (!child) continue;
        if (child) {
            child->print(indent + 1);
        }
    }
}

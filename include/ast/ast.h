#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

enum NodeType {
    NODE_COMP_UNIT,
    NODE_DECL,
    NODE_FUNC_DEF,
    NODE_BLOCK,
    NODE_STMT,
    NODE_IF,
    NODE_WHILE,
    NODE_RETURN,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_EXP,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_CALL,
    NODE_IDENTIFIER,
    NODE_INT_CONST,
    NODE_FLOAT_CONST,
    NODE_ARRAY,
    NODE_INDEX,
    NODE_TYPE  // 用于存储类型信息的节点
};

enum TypeKind {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VOID,
    TYPE_ARRAY
};

class Type {
public:
    TypeKind kind;
    std::shared_ptr<Type> base_type;
    int array_size;

    Type(TypeKind k) : kind(k), array_size(0) {}
    Type(TypeKind k, std::shared_ptr<Type> base, int size) 
        : kind(k), base_type(base), array_size(size) {}
};

class ASTNode {
public:
    NodeType type;
    std::vector<std::shared_ptr<ASTNode>> children;
    std::string value;
    int line_number;

    ASTNode(NodeType t) : type(t), line_number(0) {}
    ASTNode(NodeType t, const std::string& v) : type(t), value(v), line_number(0) {}

    void addChild(std::shared_ptr<ASTNode> child) {
        children.push_back(child);
    }

    // 接受原始指针的重载（用于 parser，节点由 node_registry 管理）
    void addChild(ASTNode* child) {
        // 使用空 deleter，因为节点的生命周期由 node_registry 管理
        children.push_back(std::shared_ptr<ASTNode>(child, [](ASTNode*){}));
    }

    void print(int indent = 0);
};

std::string nodeTypeToString(NodeType type);

#endif // AST_H

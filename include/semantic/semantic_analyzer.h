#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../ast/ast.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <iostream>

struct SemanticType {
    TypeKind kind;
    std::shared_ptr<Type> baseType;
    std::vector<int> dimensions;
    std::string returnType;
    std::vector<TypeKind> paramTypes;

    SemanticType(TypeKind k = TYPE_INT) : kind(k) {}

    bool isInt() const { return kind == TYPE_INT; }
    bool isFloat() const { return kind == TYPE_FLOAT; }
    bool isVoid() const { return kind == TYPE_VOID; }
    bool isArray() const { return kind == TYPE_ARRAY; }
    bool isNumeric() const { return kind == TYPE_INT || kind == TYPE_FLOAT; }

    std::string toString() const {
        switch (kind) {
            case TYPE_INT: return "int";
            case TYPE_FLOAT: return "float";
            case TYPE_VOID: return "void";
            case TYPE_ARRAY: return "array";
            default: return "unknown";
        }
    }
};

struct Symbol {
    std::string name;
    SemanticType type;
    bool isConst;
    bool isFunction;
    int scopeLevel;
    int offset;
    std::vector<std::string> paramNames;

    Symbol(const std::string& n, SemanticType t, bool c = false, bool func = false)
        : name(n), type(t), isConst(c), isFunction(func), scopeLevel(0), offset(0) {}
};

class Scope {
public:
    int level;
    std::map<std::string, std::shared_ptr<Symbol>> symbols;
    std::shared_ptr<Scope> parent;

    Scope(int l, std::shared_ptr<Scope> p = nullptr) 
        : level(l), parent(p) {}

    bool addSymbol(std::shared_ptr<Symbol> symbol) {
        if (symbols.find(symbol->name) != symbols.end()) {
            return false;
        }
        symbol->scopeLevel = level;
        symbols[symbol->name] = symbol;
        return true;
    }

    std::shared_ptr<Symbol> lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->lookup(name);
        }
        return nullptr;
    }

    std::shared_ptr<Symbol> lookupLocal(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        return nullptr;
    }
};

class SemanticAnalyzer {
private:
    std::shared_ptr<Scope> currentScope;
    int scopeLevel;
    bool hasError;
    std::shared_ptr<ASTNode> currentFunction;
    std::map<std::string, SemanticType> functionTypes;

    void enterScope();
    void exitScope();
    SemanticType checkNode(std::shared_ptr<ASTNode> node);
    SemanticType checkBinaryOp(std::shared_ptr<ASTNode> node);
    SemanticType checkUnaryOp(std::shared_ptr<ASTNode> node);
    SemanticType checkAssignment(std::shared_ptr<ASTNode> node);
    SemanticType checkIf(std::shared_ptr<ASTNode> node);
    SemanticType checkWhile(std::shared_ptr<ASTNode> node);
    SemanticType checkReturn(std::shared_ptr<ASTNode> node);
    SemanticType checkFunctionCall(std::shared_ptr<ASTNode> node);
    SemanticType checkArrayAccess(std::shared_ptr<ASTNode> node);
    SemanticType checkTypeCast(std::shared_ptr<ASTNode> node);
    void checkVariableDeclaration(std::shared_ptr<ASTNode> node);
    void checkFunctionDeclaration(std::shared_ptr<ASTNode> node);
    void checkBlock(std::shared_ptr<ASTNode> node);

    SemanticType getNumericResultType(SemanticType left, SemanticType right);
    SemanticType getRelationalResultType(SemanticType left, SemanticType right);
    bool isAssignable(std::shared_ptr<ASTNode> node);
    void reportError(const std::string& message, std::shared_ptr<ASTNode> node = nullptr);

public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    bool analyze(std::shared_ptr<ASTNode> root);
    bool hasErrors() const { return hasError; }

    std::shared_ptr<Scope> getCurrentScope() const { return currentScope; }
};

#endif

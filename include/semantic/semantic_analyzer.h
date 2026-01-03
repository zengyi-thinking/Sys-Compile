/**
 * @file semantic_analyzer.h
 * @brief 语义分析器(Semantic Analyzer)的头文件
 *
 * 本模块负责对抽象语法树(AST)进行语义分析，确保程序在语义上是正确的。
 *
 * ==================== 语义分析的作用 ====================
 * 语法分析只检查代码的结构是否符合语法规则，而语义分析检查代码的"意义"是否正确。
 *
 * 主要检查内容:
 * 1. 变量是否先声明后使用
 * 2. 类型是否匹配(如不能把float赋值给int)
 * 3. 函数调用参数个数和类型是否正确
 * 4. return语句的返回值类型是否与函数声明一致
 * 5. break/continue是否在循环中
 * 6. 数组索引是否为整数
 * 7. 变量/函数是否重复定义
 *
 * ==================== 符号表与作用域 ====================
 * 符号表(Symbol Table)用于记录程序中声明的所有标识符(变量、函数等)。
 * 作用域(Scope)决定了标识符的可见范围。
 *
 * 作用域层级:
 * - level 0: 全局作用域(函数外部)
 * - level 1+: 局部作用域(函数内部、代码块内部)
 */

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../ast/ast.h"
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <iostream>

/**
 * @struct SemanticType
 * @brief 语义类型信息
 *
 * 存储类型相关的语义信息，用于类型检查。
 */
struct SemanticType {
    TypeKind kind;                          ///< 类型种类(int/float/void/array)
    TypeKind elemType;                      ///< 数组元素类型(仅当kind=TYPE_ARRAY时有效)
    std::shared_ptr<Type> baseType;         ///< 数组的基类型
    std::vector<int> dimensions;            ///< 数组的维度(如 [2][3] 的 {2, 3})
    std::string returnType;                 ///< 返回类型(字符串形式，用于调试)
    std::vector<TypeKind> paramTypes;       ///< 函数参数类型列表

    /**
     * @brief 构造函数
     * @param k 类型种类，默认为int
     * @param e 数组元素类型(仅当kind=TYPE_ARRAY时使用)
     */
    SemanticType(TypeKind k = TYPE_INT, TypeKind e = TYPE_INT) : kind(k), elemType(e) {}

    /**
     * @brief 检查是否为int类型
     * @return 是int类型返回true
     */
    bool isInt() const { return kind == TYPE_INT; }

    /**
     * @brief 检查是否为float类型
     * @return 是float类型返回true
     */
    bool isFloat() const { return kind == TYPE_FLOAT; }

    /**
     * @brief 检查是否为void类型
     * @return 是void类型返回true
     */
    bool isVoid() const { return kind == TYPE_VOID; }

    /**
     * @brief 检查是否为数组类型
     * @return 是数组类型返回true
     */
    bool isArray() const { return kind == TYPE_ARRAY; }

    /**
     * @brief 检查是否为数值类型(int或float)
     * @return 是数值类型返回true
     */
    bool isNumeric() const { return kind == TYPE_INT || kind == TYPE_FLOAT; }

    /**
     * @brief 将类型转换为字符串
     * @return 类型名称字符串
     */
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

/**
 * @struct Symbol
 * @brief 符号表条目
 *
 * 表示程序中声明的一个标识符(变量、函数等)。
 */
struct Symbol {
    std::string name;                       ///< 符号名称(变量名/函数名)
    SemanticType type;                      ///< 符号的类型
    bool isConst;                           ///< 是否为常量
    bool isFunction;                        ///< 是否为函数
    int scopeLevel;                         ///< 所在作用域层级
    int offset;                             ///< 在栈中的偏移量(用于代码生成)
    std::vector<std::string> paramNames;    ///< 函数参数名称列表

    /**
     * @brief 构造函数
     * @param n 符号名称
     * @param t 符号类型
     * @param c 是否为常量
     * @param func 是否为函数
     */
    Symbol(const std::string& n, SemanticType t, bool c = false, bool func = false)
        : name(n), type(t), isConst(c), isFunction(func), scopeLevel(0), offset(0) {}
};

/**
 * @class Scope
 * @brief 作用域类
 *
 * 管理一个作用域内的所有符号。支持作用域嵌套和符号查找。
 *
 * 示例:
 * @code
 * int a;       // 全局作用域(level 0)，符号a
 * void foo() {
 *     int b;   // 局部作用域(level 1)，符号b
 *     {
 *         int c; // 嵌套作用域(level 2)，符号c
 *         // 这里可以访问a, b, c
 *     }
 *     // 这里可以访问a, b，但不能访问c
 * }
 * @endcode
 */
class Scope {
public:
    int level;                                              ///< 作用域层级
    std::map<std::string, std::shared_ptr<Symbol>> symbols; ///< 符号表(名称->符号)
    std::shared_ptr<Scope> parent;                          ///< 父作用域(用于向上查找)

    /**
     * @brief 构造函数
     * @param l 作用域层级
     * @param p 父作用域
     */
    Scope(int l, std::shared_ptr<Scope> p = nullptr)
        : level(l), parent(p) {}

    /**
     * @brief 向符号表添加一个符号
     * @param symbol 要添加的符号
     * @return 添加成功返回true，如果符号已存在返回false
     */
    bool addSymbol(std::shared_ptr<Symbol> symbol) {
        if (symbols.find(symbol->name) != symbols.end()) {
            return false;  // 符号已存在
        }
        symbol->scopeLevel = level;
        symbols[symbol->name] = symbol;
        return true;
    }

    /**
     * @brief 查找符号(支持向上查找)
     * @param name 符号名称
     * @return 找到的符号，未找到返回nullptr
     *
     * 先在当前作用域查找，若未找到则在父作用域递归查找。
     */
    std::shared_ptr<Symbol> lookup(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->lookup(name);  // 向父作用域查找
        }
        return nullptr;
    }

    /**
     * @brief 仅在当前作用域查找符号
     * @param name 符号名称
     * @return 找到的符号，未找到返回nullptr
     *
     * 不向上查找父作用域。
     */
    std::shared_ptr<Symbol> lookupLocal(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        return nullptr;
    }
};

/**
 * @class SemanticAnalyzer
 * @brief 语义分析器类
 *
 * 遍历抽象语法树(AST)，进行语义检查，维护符号表和作用域信息。
 */
class SemanticAnalyzer {
private:
    std::shared_ptr<Scope> currentScope;        ///< 当前作用域
    int scopeLevel;                             ///< 当前作用域层级
    bool hasError;                              ///< 是否发现语义错误
    std::shared_ptr<ASTNode> currentFunction;   ///< 当前正在分析的函数
    std::map<std::string, SemanticType> functionTypes; ///< 函数名到返回类型的映射

    // ============ 作用域管理 ============

    /**
     * @brief 进入新的作用域
     */
    void enterScope();

    /**
     * @brief 退出当前作用域
     */
    void exitScope();

    // ============ AST节点检查函数 ============

    /**
     * @brief 检查AST节点(分发函数)
     * @param node 要检查的节点
     * @return 节点的类型
     */
    SemanticType checkNode(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查二元运算表达式
     * @param node 二元运算节点
     * @return 运算结果的类型
     */
    SemanticType checkBinaryOp(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查一元运算表达式
     * @param node 一元运算节点
     * @return 运算结果的类型
     */
    SemanticType checkUnaryOp(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查赋值表达式
     * @param node 赋值节点
     * @return 赋值的类型
     */
    SemanticType checkAssignment(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查if语句
     * @param node if语句节点
     * @return void类型
     */
    SemanticType checkIf(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查while语句
     * @param node while语句节点
     * @return void类型
     */
    SemanticType checkWhile(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查return语句
     * @param node return语句节点
     * @return void类型
     */
    SemanticType checkReturn(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查函数调用
     * @param node 函数调用节点
     * @return 函数的返回类型
     */
    SemanticType checkFunctionCall(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查数组访问
     * @param node 数组索引节点
     * @return 数组元素的类型
     */
    SemanticType checkArrayAccess(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查类型转换
     * @param node 类型转换节点
     * @return 转换后的类型
     */
    SemanticType checkTypeCast(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查变量声明
     * @param node 变量声明节点
     */
    void checkVariableDeclaration(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查函数声明
     * @param node 函数定义节点
     */
    void checkFunctionDeclaration(std::shared_ptr<ASTNode> node);

    /**
     * @brief 检查代码块
     * @param node 代码块节点
     */
    void checkBlock(std::shared_ptr<ASTNode> node);

    // ============ 辅助函数 ============

    /**
     * @brief 获取数值运算的结果类型
     * @param left 左操作数类型
     * @param right 右操作数类型
     * @return 结果类型(float运算产生float，否则int)
     */
    SemanticType getNumericResultType(SemanticType left, SemanticType right);

    /**
     * @brief 获取关系运算的结果类型
     * @param left 左操作数类型
     * @param right 右操作数类型
     * @return 结果类型(总是int)
     */
    SemanticType getRelationalResultType(SemanticType left, SemanticType right);

    /**
     * @brief 检查节点是否可赋值(是否为左值)
     * @param node 要检查的节点
     * @return 可赋值返回true
     */
    bool isAssignable(std::shared_ptr<ASTNode> node);

    /**
     * @brief 报告语义错误
     * @param message 错误信息
     * @param node 相关的AST节点(用于获取行号)
     */
    void reportError(const std::string& message, std::shared_ptr<ASTNode> node = nullptr);

public:
    /**
     * @brief 构造函数
     */
    SemanticAnalyzer();

    /**
     * @brief 析构函数
     */
    ~SemanticAnalyzer();

    /**
     * @brief 对AST进行语义分析
     * @param root AST的根节点
     * @return 分析成功返回true，发现错误返回false
     */
    bool analyze(std::shared_ptr<ASTNode> root);

    /**
     * @brief 检查是否有语义错误
     * @return 有错误返回true
     */
    bool hasErrors() const { return hasError; }

    /**
     * @brief 获取当前作用域
     * @return 当前作用域的指针
     */
    std::shared_ptr<Scope> getCurrentScope() const { return currentScope; }
};

#endif // SEMANTIC_ANALYZER_H

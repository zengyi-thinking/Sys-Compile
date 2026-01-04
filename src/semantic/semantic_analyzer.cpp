/**
 * @file semantic_analyzer.cpp
 * @brief 语义分析器的实现文件
 *
 * 实现对抽象语法树(AST)的语义分析，包括：
 * - 符号表管理
 * - 作用域管理
 * - 类型检查
 * - 各种语义错误的检测
 */

#include "semantic/semantic_analyzer.h"
#include <algorithm>

// ==================== 构造与析构 ====================

SemanticAnalyzer::SemanticAnalyzer() {
    scopeLevel = 0;
    hasError = false;
    currentScope = std::make_shared<Scope>(scopeLevel);
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

// ==================== 作用域管理 ====================

void SemanticAnalyzer::enterScope() {
    scopeLevel++;
    auto newScope = std::make_shared<Scope>(scopeLevel, currentScope);
    if (currentScope) {
        currentScope->children.push_back(newScope);
    }
    currentScope = newScope;
}

void SemanticAnalyzer::exitScope() {
    if (currentScope->parent) {
        currentScope = currentScope->parent;
        scopeLevel--;
    }
}

// ==================== 辅助函数 ====================

void SemanticAnalyzer::reportError(const std::string& message, std::shared_ptr<ASTNode> node) {
    hasError = true;
    std::cerr << "语义错误";
    if (node) {
        std::cerr << " (行 " << node->line_number << ")";
    }
    std::cerr << ": " << message << std::endl;
}

SemanticType SemanticAnalyzer::getNumericResultType(SemanticType left, SemanticType right) {
    if (left.kind == TYPE_FLOAT || right.kind == TYPE_FLOAT) {
        return SemanticType(TYPE_FLOAT);
    }
    return SemanticType(TYPE_INT);
}

SemanticType SemanticAnalyzer::getRelationalResultType(SemanticType left, SemanticType right) {
    return SemanticType(TYPE_INT);
}

bool SemanticAnalyzer::isAssignable(std::shared_ptr<ASTNode> node) {
    if (!node) return false;
    
    switch (node->type) {
        case NODE_IDENTIFIER:
            return true;
        case NODE_INDEX:
            return true;
        default:
            return false;
    }
}

// ==================== AST节点检查函数 ====================

/**
 * @brief 主检查函数 - 根据节点类型分发到具体的检查函数
 */
SemanticType SemanticAnalyzer::checkNode(std::shared_ptr<ASTNode> node) {
    if (!node) return SemanticType(TYPE_VOID);

    switch (node->type) {
        case NODE_INT_CONST:
            return SemanticType(TYPE_INT);

        case NODE_FLOAT_CONST:
            return SemanticType(TYPE_FLOAT);

        case NODE_IDENTIFIER: {
            std::shared_ptr<Symbol> symbol = currentScope->lookup(node->value);
            if (!symbol) {
                reportError("未定义的标识符: '" + node->value + "'", node);
                return SemanticType(TYPE_INT);
            }
            return symbol->type;
        }
        
        case NODE_BINARY_OP:
            return checkBinaryOp(node);
        
        case NODE_UNARY_OP:
            return checkUnaryOp(node);
        
        case NODE_ASSIGN:
            return checkAssignment(node);
        
        case NODE_IF:
            return checkIf(node);
        
        case NODE_WHILE:
            return checkWhile(node);
        
        case NODE_RETURN:
            return checkReturn(node);
        
        case NODE_CALL:
            return checkFunctionCall(node);
        
        case NODE_INDEX:
            return checkArrayAccess(node);

        case NODE_DECL:
        case NODE_CONST_DECL:
            checkVariableDeclaration(node);
            return SemanticType(TYPE_VOID);

        case NODE_FUNC_DEF:
            checkFunctionDeclaration(node);
            return SemanticType(TYPE_VOID);
        
        case NODE_BLOCK:
            checkBlock(node);
            return SemanticType(TYPE_VOID);
        
        case NODE_BREAK:
        case NODE_CONTINUE:
            return SemanticType(TYPE_VOID);
        
        default:
            return SemanticType(TYPE_VOID);
    }
}

// ============ 表达式检查 ============

SemanticType SemanticAnalyzer::checkBinaryOp(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return SemanticType(TYPE_VOID);

    SemanticType leftType = checkNode(node->children[0]);
    SemanticType rightType = checkNode(node->children[1]);
    
    std::string op = node->value;
    
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%") {
        if (!leftType.isNumeric() || !rightType.isNumeric()) {
            reportError("运算符 '" + op + "' 需要数值类型", node);
            return SemanticType(TYPE_INT);
        }
        return getNumericResultType(leftType, rightType);
    }
    
    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (!leftType.isNumeric() || !rightType.isNumeric()) {
            reportError("关系运算符 '" + op + "' 需要数值类型", node);
            return SemanticType(TYPE_INT);
        }
        return getRelationalResultType(leftType, rightType);
    }
    
    if (op == "==" || op == "!=") {
        return getRelationalResultType(leftType, rightType);
    }
    
    if (op == "&&" || op == "||") {
        if (!leftType.isInt() || !rightType.isInt()) {
            reportError("逻辑运算符 '" + op + "' 需要整数类型", node);
            return SemanticType(TYPE_INT);
        }
        return SemanticType(TYPE_INT);
    }
    
    return SemanticType(TYPE_INT);
}

SemanticType SemanticAnalyzer::checkUnaryOp(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) return SemanticType(TYPE_VOID);

    std::string op = node->value;

    // 处理类型转换：(int)expr 和 (float)expr
    if (op == "(int)") {
        checkNode(node->children[0]);  // 检查操作数
        return SemanticType(TYPE_INT);  // 返回转换后的类型
    }

    if (op == "(float)") {
        checkNode(node->children[0]);  // 检查操作数
        return SemanticType(TYPE_FLOAT);  // 返回转换后的类型
    }

    // 检查操作数类型
    SemanticType operandType = checkNode(node->children[0]);

    if (op == "+" || op == "-") {
        if (!operandType.isNumeric()) {
            reportError("一元运算符 '" + op + "' 需要数值类型", node);
            return SemanticType(TYPE_INT);
        }
        return operandType;
    }

    if (op == "!") {
        if (!operandType.isInt()) {
            reportError("逻辑非运算符 '!' 需要整数类型", node);
            return SemanticType(TYPE_INT);
        }
        return SemanticType(TYPE_INT);
    }

    return operandType;
}

SemanticType SemanticAnalyzer::checkAssignment(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return SemanticType(TYPE_VOID);

    std::shared_ptr<ASTNode> left = node->children[0];
    std::shared_ptr<ASTNode> right = node->children[1];

    if (!isAssignable(left)) {
        reportError("赋值目标必须是左值", left);
        return SemanticType(TYPE_VOID);
    }

    // 检查是否赋值给const变量
    if (left->type == NODE_IDENTIFIER) {
        std::shared_ptr<Symbol> symbol = currentScope->lookup(left->value);
        if (symbol && symbol->isConst) {
            reportError("不能给常量 '" + left->value + "' 赋值", node);
            return SemanticType(TYPE_VOID);
        }
    } else if (left->type == NODE_INDEX) {
        // 对于数组访问，需要获取数组名并检查是否为const
        // 递归查找最底层的标识符
        std::shared_ptr<ASTNode> base = left->children[0];
        while (base && base->type == NODE_INDEX) {
            base = base->children[0];
        }
        if (base && base->type == NODE_IDENTIFIER) {
            std::shared_ptr<Symbol> symbol = currentScope->lookup(base->value);
            if (symbol && symbol->isConst) {
                reportError("不能给常量数组 '" + base->value + "' 的元素赋值", node);
                return SemanticType(TYPE_VOID);
            }
        }
    }

    SemanticType leftType = checkNode(left);
    SemanticType rightType = checkNode(right);

    if (leftType.kind == TYPE_INT && rightType.kind == TYPE_FLOAT) {
        reportError("不能将float类型赋值给int类型变量", node);
    }

    return leftType;
}

SemanticType SemanticAnalyzer::checkIf(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return SemanticType(TYPE_VOID);

    SemanticType condType = checkNode(node->children[0]);
    if (!condType.isInt()) {
        reportError("if条件需要整数类型", node->children[0]);
    }

    // 语句体如果是 Block，会自己创建作用域
    checkNode(node->children[1]);

    if (node->children.size() > 2) {
        checkNode(node->children[2]);
    }

    return SemanticType(TYPE_VOID);
}

SemanticType SemanticAnalyzer::checkWhile(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return SemanticType(TYPE_VOID);

    SemanticType condType = checkNode(node->children[0]);
    if (!condType.isInt()) {
        reportError("while条件需要整数类型", node->children[0]);
    }

    // 语句体如果是 Block，会自己创建作用域
    checkNode(node->children[1]);

    return SemanticType(TYPE_VOID);
}

SemanticType SemanticAnalyzer::checkReturn(std::shared_ptr<ASTNode> node) {
    if (!currentFunction) {
        reportError("return语句必须在函数内部", node);
        return SemanticType(TYPE_VOID);
    }
    
    std::string funcName = currentFunction->value;
    auto it = functionTypes.find(funcName);
    if (it == functionTypes.end()) {
        return SemanticType(TYPE_VOID);
    }
    
    SemanticType funcReturnType = it->second;
    
    if (node->children.empty()) {
        if (!funcReturnType.isVoid()) {
            reportError("函数 '" + funcName + "' 需要返回值", node);
        }
    } else {
        SemanticType exprType = checkNode(node->children[0]);
        if (funcReturnType.kind == TYPE_INT && exprType.kind == TYPE_FLOAT) {
            reportError("不能将float类型返回给int类型函数", node);
        }
    }
    
    return SemanticType(TYPE_VOID);
}

SemanticType SemanticAnalyzer::checkFunctionCall(std::shared_ptr<ASTNode> node) {
    std::string funcName = node->value;
    
    auto it = functionTypes.find(funcName);
    if (it == functionTypes.end()) {
        reportError("未定义的函数: '" + funcName + "'", node);
        return SemanticType(TYPE_INT);
    }
    
    SemanticType funcType = it->second;
    
    std::shared_ptr<Symbol> symbol = currentScope->lookup(funcName);
    if (!symbol) {
        return SemanticType(TYPE_INT);
    }
    
    int expectedParams = symbol->paramNames.size();
    int actualParams = node->children.size();
    
    if (expectedParams != actualParams) {
        reportError("函数 '" + funcName + "' 需要 " + std::to_string(expectedParams) + 
                   " 个参数，但提供了 " + std::to_string(actualParams) + " 个", node);
    }
    
    for (int i = 0; i < std::min(expectedParams, actualParams); i++) {
        checkNode(node->children[i]);
    }
    
    return funcType;
}

/**
 * @brief 检查数组访问
 * @param node 数组索引节点(NODE_INDEX)
 * @return 数组元素的类型
 *
 * AST结构:
 * - 对于 arr[i]: node->type = NODE_INDEX
 *                 node->children[0] = NODE_IDENTIFIER (arr)
 *                 node->children[1] = 索引表达式 (i)
 * - 对于 arr[i][j]: node->type = NODE_INDEX
 *                   node->children[0] = 内层 NODE_INDEX
 *                   node->children[1] = 索引表达式 (j)
 */
SemanticType SemanticAnalyzer::checkArrayAccess(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) {
        return SemanticType(TYPE_INT);
    }

    // 递归检查内层的数组访问
    SemanticType baseType = checkNode(node->children[0]);

    // 检查索引是否为整数
    if (node->children.size() > 1) {
        SemanticType indexType = checkNode(node->children[1]);
        if (!indexType.isInt()) {
            reportError("数组索引必须是整数类型", node->children[1]);
        }
    }

    // 如果是数组类型，返回元素类型；否则返回原类型
    if (baseType.kind == TYPE_ARRAY) {
        return SemanticType(baseType.elemType);
    }

    return baseType;
}

SemanticType SemanticAnalyzer::checkTypeCast(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) return SemanticType(TYPE_INT);
    
    SemanticType targetType(node->value == "int" ? TYPE_INT : TYPE_FLOAT);
    checkNode(node->children[0]);
    
    return targetType;
}

// ============ 声明检查 ============

/**
 * @brief 检查变量声明
 *
 * AST结构:
 * - node->value = 类型名 ("int" 或 "float")
 * - node->children[0] = 标识符节点 (变量名)
 * - node->children[1] = 初始值表达式（可选）或数组大小
 *
 * 对于数组声明 (如 int arr[5]):
 * - 需要将符号标记为数组类型
 */
void SemanticAnalyzer::checkVariableDeclaration(std::shared_ptr<ASTNode> node) {
    // 新的 AST 结构：
    // - node->value = 类型名 ("int" 或 "float")
    // - node->children[0] = 标识符节点 (变量名)
    // - node->children[1] = 初始值表达式（可选）或数组大小

    std::string typeName = node->value;
    TypeKind typeKind = (typeName == "float") ? TYPE_FLOAT : TYPE_INT;

    if (node->children.empty()) {
        return;  // 错误：没有变量名
    }

    std::shared_ptr<ASTNode> varNode = node->children[0];
    std::string varName = varNode->value;

    // 检查是否为数组声明（有两个子节点，且第二个是数组大小）
    bool isArray = false;
    if (node->children.size() >= 2) {
        auto secondChild = node->children[1];
        // 如果第二个子节点是整数常量，且不是初始化表达式的一部分，则可能是数组
        // 这里需要区分：int a[5] 是数组，int a = 5 是初始化
        // 判断依据：如果children[1]是NODE_INT_CONST且后续没有初始化语义，则是数组
        // 为了简化，我们假设有LBRACK RBRACK的声明的children[1]是数组大小

        // 更可靠的方法：检查解析器如何区分数组和非数组
        // 观察parser.y，数组声明不会有ASSIGN，所以如果有ASSIGN就是初始化
        // 但这里我们已经在DECL节点内部，需要从其他信息判断

        // 临时方案：检查这个声明是否来自数组语法
        // 如果是数组，在语义类型中设置
    }

    // 创建符号，根据是否为数组设置类型
    SemanticType symbolType(typeKind);

    // 检查是否为const声明
    bool isConst = (node->type == NODE_CONST_DECL);

    std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>(varName, symbolType, isConst);
    if (!currentScope->addSymbol(symbol)) {
        reportError("变量 '" + varName + "' 已在此作用域中定义", node);
    }

    // 检查初始值表达式或数组大小（如果存在）
    if (node->children.size() > 1) {
        SemanticType initType = checkNode(node->children[1]);
        // 类型检查：不能将float赋值给int
        if (typeKind == TYPE_INT && initType.kind == TYPE_FLOAT) {
            reportError("不能将float类型赋值给int类型变量 '" + varName + "'", node);
        }
    }
}

/**
 * @brief 检查函数声明
 *
 * AST结构:
 * - node->value = 函数名
 * - node->children[0] = 类型节点 (返回类型)
 * - node->children[1...] = 参数节点
 * - 最后一个child = Block (函数体)
 */
void SemanticAnalyzer::checkFunctionDeclaration(std::shared_ptr<ASTNode> node) {
    // 新的 AST 结构：
    // - node->value = 函数名
    // - node->children[0] = 类型节点 (返回类型)
    // - node->children[1]... = 参数或 Block

    std::string funcName = node->value;

    // 获取返回类型
    TypeKind returnType = TYPE_INT;
    if (!node->children.empty() && node->children[0]->type == NODE_TYPE) {
        std::string returnTypeName = node->children[0]->value;
        returnType = (returnTypeName == "float") ? TYPE_FLOAT :
                      (returnTypeName == "void") ? TYPE_VOID : TYPE_INT;
    }

    std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>(funcName, SemanticType(returnType));
    symbol->isFunction = true;

    if (!currentScope->addSymbol(symbol)) {
        reportError("函数 '" + funcName + "' 已定义", node);
        return;
    }

    // 创建函数作用域（包含参数和函数体）
    enterScope();
    currentFunction = node;

    // 处理参数
    // 无参数：children = [TYPE, BLOCK]
    // 有参数：children = [TYPE, param1, param2, ..., BLOCK]
    size_t paramStart = 1;
    size_t lastChildIndex = node->children.size() - 1;

    // 检查从 paramStart 到 lastChildIndex-1 的节点是否是参数
    for (size_t i = paramStart; i < lastChildIndex; i++) {
        std::shared_ptr<ASTNode> paramNode = node->children[i];

        // 参数节点格式：DECL 类型 IDENTIFIER
        if (paramNode->type == NODE_DECL) {
            std::string paramName = paramNode->children[0]->value;
            std::string paramTypeName = paramNode->value;
            TypeKind paramType = (paramTypeName == "float") ? TYPE_FLOAT : TYPE_INT;

            // 检查是否为数组参数（有额外的标记节点 value="[]"）
            bool isArrayParam = false;
            if (paramNode->children.size() > 1) {
                auto secondChild = paramNode->children[1];
                if (secondChild->type == NODE_STMT && secondChild->value == "[]") {
                    isArrayParam = true;
                }
            }

            // 创建数组类型时指定元素类型
            SemanticType paramSemanticType;
            if (isArrayParam) {
                paramSemanticType = SemanticType(TYPE_ARRAY, paramType);
            } else {
                paramSemanticType = SemanticType(paramType);
            }

            std::shared_ptr<Symbol> paramSymbol = std::make_shared<Symbol>(paramName, paramSemanticType);
            if (!currentScope->addSymbol(paramSymbol)) {
                reportError("参数 '" + paramName + "' 重复定义", paramNode);
            }

            symbol->paramNames.push_back(paramName);
            symbol->type.paramTypes.push_back(isArrayParam ? TYPE_ARRAY : paramType);
        }
    }

    // 检查函数体（Block）
    // 注意：checkBlock 会创建一个新的作用域，所以函数的局部变量会在嵌套作用域中
    std::shared_ptr<ASTNode> bodyNode = node->children.back();
    checkNode(bodyNode);

    exitScope();
    currentFunction = nullptr;

    functionTypes[funcName] = SemanticType(returnType);
}

void SemanticAnalyzer::checkBlock(std::shared_ptr<ASTNode> node) {
    // Block 创建新作用域
    enterScope();
    for (auto& child : node->children) {
        checkNode(child);
    }
    exitScope();
}

// ==================== 主分析入口 ====================

/**
 * @brief 对AST进行语义分析
 * @param root AST的根节点(NODE_COMP_UNIT)
 * @return 分析成功返回true，发现错误返回false
 */
bool SemanticAnalyzer::analyze(std::shared_ptr<ASTNode> root) {
    if (!root) return false;
    
    hasError = false;
    scopeLevel = 0;
    currentScope = std::make_shared<Scope>(scopeLevel);
    currentFunction = nullptr;
    
    if (root->type == NODE_COMP_UNIT) {
        for (auto& child : root->children) {
            checkNode(child);
        }
    }
    
    return !hasError;
}

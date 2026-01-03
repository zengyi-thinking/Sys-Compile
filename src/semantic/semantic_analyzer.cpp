#include "semantic/semantic_analyzer.h"
#include <algorithm>

SemanticAnalyzer::SemanticAnalyzer() {
    scopeLevel = 0;
    hasError = false;
    currentScope = std::make_shared<Scope>(scopeLevel);
}

SemanticAnalyzer::~SemanticAnalyzer() {
}

void SemanticAnalyzer::enterScope() {
    scopeLevel++;
    currentScope = std::make_shared<Scope>(scopeLevel, currentScope);
}

void SemanticAnalyzer::exitScope() {
    if (currentScope->parent) {
        currentScope = currentScope->parent;
        scopeLevel--;
    }
}

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
    
    SemanticType operandType = checkNode(node->children[0]);
    std::string op = node->value;
    
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

SemanticType SemanticAnalyzer::checkArrayAccess(std::shared_ptr<ASTNode> node) {
    std::string arrayName = node->value;
    
    std::shared_ptr<Symbol> symbol = currentScope->lookup(arrayName);
    if (!symbol) {
        reportError("未定义的数组: '" + arrayName + "'", node);
        return SemanticType(TYPE_INT);
    }
    
    if (!symbol->type.isArray()) {
        reportError("'" + arrayName + "' 不是数组类型", node);
        return SemanticType(TYPE_INT);
    }
    
    for (size_t i = 0; i < node->children.size(); i++) {
        SemanticType indexType = checkNode(node->children[i]);
        if (!indexType.isInt()) {
            reportError("数组索引必须是整数类型", node->children[i]);
        }
    }
    
    if (!symbol->type.baseType) {
        return SemanticType(TYPE_INT);
    }
    
    switch (symbol->type.baseType->kind) {
        case TYPE_INT:
            return SemanticType(TYPE_INT);
        case TYPE_FLOAT:
            return SemanticType(TYPE_FLOAT);
        default:
            return SemanticType(TYPE_INT);
    }
}

SemanticType SemanticAnalyzer::checkTypeCast(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) return SemanticType(TYPE_INT);
    
    SemanticType targetType(node->value == "int" ? TYPE_INT : TYPE_FLOAT);
    checkNode(node->children[0]);
    
    return targetType;
}

void SemanticAnalyzer::checkVariableDeclaration(std::shared_ptr<ASTNode> node) {
    // 新的 AST 结构：
    // - node->value = 类型名 ("int" 或 "float")
    // - node->children[0] = 标识符节点 (变量名)
    // - node->children[1] = 初始值表达式（可选）
    std::string typeName = node->value;
    TypeKind typeKind = (typeName == "float") ? TYPE_FLOAT : TYPE_INT;

    if (node->children.empty()) {
        return;  // 错误：没有变量名
    }

    std::shared_ptr<ASTNode> varNode = node->children[0];
    std::string varName = varNode->value;

    std::shared_ptr<Symbol> symbol = std::make_shared<Symbol>(varName, SemanticType(typeKind));
    if (!currentScope->addSymbol(symbol)) {
        reportError("变量 '" + varName + "' 已在此作用域中定义", node);
    }

    // 检查初始值表达式（如果存在）
    if (node->children.size() > 1) {
        SemanticType initType = checkNode(node->children[1]);
        // 类型检查：不能将float赋值给int
        if (typeKind == TYPE_INT && initType.kind == TYPE_FLOAT) {
            reportError("不能将float类型赋值给int类型变量 '" + varName + "'", node);
        }
    }
}

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

            SemanticType paramSemanticType(paramType);
            std::shared_ptr<Symbol> paramSymbol = std::make_shared<Symbol>(paramName, paramSemanticType);
            if (!currentScope->addSymbol(paramSymbol)) {
                reportError("参数 '" + paramName + "' 重复定义", paramNode);
            }

            symbol->paramNames.push_back(paramName);
            symbol->type.paramTypes.push_back(paramType);
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

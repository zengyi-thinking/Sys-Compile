#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "../ast/ast.h"
#include "semantic/semantic_analyzer.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

enum class OpCode {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    NEG,
    AND,
    OR,
    NOT,
    
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    
    ASSIGN,
    
    PARAM,
    CALL,
    RETURN,
    
    LABEL,
    JUMP,
    JZ,
    JNZ,
    
    ALLOC,
    
    LOAD,
    STORE,
    
    CAST_INT,
    CAST_FLOAT
};

class Instruction {
public:
    OpCode op;
    std::string result;
    std::string arg1;
    std::string arg2;
    std::string label;

    Instruction(OpCode o) : op(o) {}
    Instruction(OpCode o, const std::string& r) : op(o), result(r) {}
    Instruction(OpCode o, const std::string& r, const std::string& a1) 
        : op(o), result(r), arg1(a1) {}
    Instruction(OpCode o, const std::string& r, const std::string& a1, const std::string& a2) 
        : op(o), result(r), arg1(a1), arg2(a2) {}
    Instruction(OpCode o, const std::string& l, bool isLabel) 
        : op(o), label(l) {}

    std::string toString() const;
};

class BasicBlock {
public:
    std::string label;
    std::vector<std::shared_ptr<Instruction>> instructions;
    std::shared_ptr<BasicBlock> trueBranch;
    std::shared_ptr<BasicBlock> falseBranch;
    std::shared_ptr<BasicBlock> nextBlock;

    BasicBlock(const std::string& l) : label(l) {}

    void addInstruction(std::shared_ptr<Instruction> inst) {
        instructions.push_back(inst);
    }

    std::string toString() const;
};

class Function {
public:
    std::string name;
    std::shared_ptr<BasicBlock> entryBlock;
    std::vector<std::string> params;
    std::string returnType;
    std::map<std::string, int> localVarOffsets;
    int stackSize;

    Function(const std::string& n) : name(n), stackSize(0) {}

    std::string toString() const;
};

class CodeGenerator {
private:
    std::map<std::string, std::shared_ptr<Function>> functions;
    std::shared_ptr<Function> currentFunction;
    std::shared_ptr<BasicBlock> currentBlock;
    int tempCounter;
    int labelCounter;
    int stackOffset;
    std::shared_ptr<Scope> globalScope;

    std::string newTemp();
    std::string newLabel();
    void emit(std::shared_ptr<Instruction> inst);
    
    std::shared_ptr<BasicBlock> newBasicBlock(const std::string& prefix = "");
    void setBlock(std::shared_ptr<BasicBlock> block);
    
    std::string generateExpr(std::shared_ptr<ASTNode> node);
    void generateStmt(std::shared_ptr<ASTNode> node);
    void generateBlock(std::shared_ptr<ASTNode> node);
    void generateIf(std::shared_ptr<ASTNode> node);
    void generateWhile(std::shared_ptr<ASTNode> node);
    void generateReturn(std::shared_ptr<ASTNode> node);
    void generateFunction(std::shared_ptr<ASTNode> node);
    void generateDecl(std::shared_ptr<ASTNode> node);
    void generateAssignment(std::shared_ptr<ASTNode> node);
    void generateCall(std::shared_ptr<ASTNode> node);
    std::string generateArrayAccess(std::shared_ptr<ASTNode> node);

    std::string opToString(OpCode op);
    std::string typeToPrefix(TypeKind kind);

public:
    CodeGenerator(std::shared_ptr<Scope> scope);
    ~CodeGenerator();

    void generate(std::shared_ptr<ASTNode> root);
    std::string getGeneratedCode() const;
    
    const std::map<std::string, std::shared_ptr<Function>>& getFunctions() const {
        return functions;
    }
};

#endif

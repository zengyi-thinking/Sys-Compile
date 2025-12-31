#include "codegen/code_generator.h"
#include <sstream>
#include <algorithm>
#include <set>

std::string Instruction::toString() const {
    std::ostringstream oss;
    
    switch (op) {
        case OpCode::ADD: oss << result << " = " << arg1 << " + " << arg2; break;
        case OpCode::SUB: oss << result << " = " << arg1 << " - " << arg2; break;
        case OpCode::MUL: oss << result << " = " << arg1 << " * " << arg2; break;
        case OpCode::DIV: oss << result << " = " << arg1 << " / " << arg2; break;
        case OpCode::MOD: oss << result << " = " << arg1 << " % " << arg2; break;
        case OpCode::NEG: oss << result << " = -" << arg1; break;
        case OpCode::AND: oss << result << " = " << arg1 << " && " << arg2; break;
        case OpCode::OR: oss << result << " = " << arg1 << " || " << arg2; break;
        case OpCode::NOT: oss << result << " = !" << arg1; break;
        
        case OpCode::EQ: oss << result << " = " << arg1 << " == " << arg2; break;
        case OpCode::NE: oss << result << " = " << arg1 << " != " << arg2; break;
        case OpCode::LT: oss << result << " = " << arg1 << " < " << arg2; break;
        case OpCode::LE: oss << result << " = " << arg1 << " <= " << arg2; break;
        case OpCode::GT: oss << result << " = " << arg1 << " > " << arg2; break;
        case OpCode::GE: oss << result << " = " << arg1 << " >= " << arg2; break;
        
        case OpCode::ASSIGN: oss << result << " = " << arg1; break;
        
        case OpCode::PARAM: oss << "param " << arg1; break;
        case OpCode::CALL: oss << result << " = call " << arg1; break;
        case OpCode::RETURN: 
            if (!result.empty()) oss << "return " << result;
            else oss << "return";
            break;
        
        case OpCode::LABEL: oss << label << ":"; break;
        case OpCode::JUMP: oss << "jump " << arg1; break;
        case OpCode::JZ: oss << "if " << arg1 << " == 0 goto " << arg2; break;
        case OpCode::JNZ: oss << "if " << arg1 << " != 0 goto " << arg2; break;
        
        case OpCode::ALLOC: oss << result << " = alloc " << arg1; break;
        
        case OpCode::LOAD: oss << result << " = *" << arg1; break;
        case OpCode::STORE: oss << "*" << result << " = " << arg1; break;
        
        case OpCode::CAST_INT: oss << result << " = (int)" << arg1; break;
        case OpCode::CAST_FLOAT: oss << result << " = (float)" << arg1; break;
    }
    
    return oss.str();
}

std::string BasicBlock::toString() const {
    std::ostringstream oss;
    oss << label << ":" << std::endl;
    for (const auto& inst : instructions) {
        oss << "    " << inst->toString() << std::endl;
    }
    return oss.str();
}

std::string Function::toString() const {
    std::ostringstream oss;
    oss << "function " << name << "(";
    for (size_t i = 0; i < params.size(); i++) {
        if (i > 0) oss << ", ";
        oss << params[i];
    }
    oss << ")" << std::endl;
    
    std::shared_ptr<BasicBlock> block = entryBlock;
    std::set<std::shared_ptr<BasicBlock>> visited;
    std::vector<std::shared_ptr<BasicBlock>> queue;
    
    if (block) {
        queue.push_back(block);
        while (!queue.empty()) {
            block = queue.back();
            queue.pop_back();
            
            if (visited.find(block) != visited.end()) continue;
            visited.insert(block);
            
            oss << block->toString();
            
            if (block->trueBranch && visited.find(block->trueBranch) == visited.end()) {
                queue.push_back(block->trueBranch);
            }
            if (block->falseBranch && visited.find(block->falseBranch) == visited.end()) {
                queue.push_back(block->falseBranch);
            }
            if (block->nextBlock && visited.find(block->nextBlock) == visited.end()) {
                queue.push_back(block->nextBlock);
            }
        }
    }
    
    return oss.str();
}

CodeGenerator::CodeGenerator(std::shared_ptr<Scope> scope) 
    : globalScope(scope), tempCounter(0), labelCounter(0), stackOffset(0) {
}

CodeGenerator::~CodeGenerator() {
}

std::string CodeGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

std::string CodeGenerator::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

std::shared_ptr<BasicBlock> CodeGenerator::newBasicBlock(const std::string& prefix) {
    std::string label = prefix.empty() ? newLabel() : prefix;
    return std::make_shared<BasicBlock>(label);
}

void CodeGenerator::setBlock(std::shared_ptr<BasicBlock> block) {
    currentBlock = block;
}

void CodeGenerator::emit(std::shared_ptr<Instruction> inst) {
    if (currentBlock) {
        currentBlock->addInstruction(inst);
    }
}

std::string CodeGenerator::generateExpr(std::shared_ptr<ASTNode> node) {
    if (!node) return "";
    
    switch (node->type) {
        case NODE_INT_CONST:
            return node->value;
        
        case NODE_FLOAT_CONST:
            return node->value;
        
        case NODE_IDENTIFIER:
            return node->value;
        
        case NODE_BINARY_OP: {
            std::string op = node->value;
            std::string left = generateExpr(node->children[0]);
            std::string right = generateExpr(node->children[1]);
            std::string result = newTemp();
            
            OpCode opcode;
            if (op == "+") opcode = OpCode::ADD;
            else if (op == "-") opcode = OpCode::SUB;
            else if (op == "*") opcode = OpCode::MUL;
            else if (op == "/") opcode = OpCode::DIV;
            else if (op == "%") opcode = OpCode::MOD;
            else if (op == "&&") opcode = OpCode::AND;
            else if (op == "||") opcode = OpCode::OR;
            else if (op == "==") opcode = OpCode::EQ;
            else if (op == "!=") opcode = OpCode::NE;
            else if (op == "<") opcode = OpCode::LT;
            else if (op == "<=") opcode = OpCode::LE;
            else if (op == ">") opcode = OpCode::GT;
            else if (op == ">=") opcode = OpCode::GE;
            else opcode = OpCode::ADD;
            
            emit(std::make_shared<Instruction>(opcode, result, left, right));
            return result;
        }
        
        case NODE_UNARY_OP: {
            std::string op = node->value;
            std::string operand = generateExpr(node->children[0]);
            std::string result = newTemp();
            
            OpCode opcode;
            if (op == "-") opcode = OpCode::NEG;
            else if (op == "!") opcode = OpCode::NOT;
            else opcode = OpCode::NEG;
            
            emit(std::make_shared<Instruction>(opcode, result, operand));
            return result;
        }
        
        case NODE_CALL:
            generateCall(node);
            return "";
        
        case NODE_INDEX:
            return generateArrayAccess(node);
        
        default:
            return "";
    }
}

void CodeGenerator::generateStmt(std::shared_ptr<ASTNode> node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_BLOCK:
            generateBlock(node);
            break;
        
        case NODE_IF:
            generateIf(node);
            break;
        
        case NODE_WHILE:
            generateWhile(node);
            break;
        
        case NODE_RETURN:
            generateReturn(node);
            break;
        
        case NODE_ASSIGN:
            generateAssignment(node);
            break;
        
        case NODE_DECL:
            generateDecl(node);
            break;
        
        case NODE_CALL:
            generateCall(node);
            break;
        
        default:
            generateExpr(node);
            break;
    }
}

void CodeGenerator::generateBlock(std::shared_ptr<ASTNode> node) {
    for (auto& child : node->children) {
        generateStmt(child);
    }
}

void CodeGenerator::generateIf(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;
    
    std::string cond = generateExpr(node->children[0]);
    std::string trueLabel = newLabel();
    std::string falseLabel = newLabel();
    std::string endLabel = newLabel();
    
    emit(std::make_shared<Instruction>(OpCode::JZ, cond, falseLabel));
    
    std::shared_ptr<BasicBlock> trueBlock = newBasicBlock(trueLabel);
    trueBlock->nextBlock = currentBlock->nextBlock;
    setBlock(trueBlock);
    generateStmt(node->children[1]);
    emit(std::make_shared<Instruction>(OpCode::JUMP, endLabel));
    
    if (node->children.size() > 2) {
        std::shared_ptr<BasicBlock> falseBlock = newBasicBlock(falseLabel);
        falseBlock->nextBlock = trueBlock->nextBlock;
        setBlock(falseBlock);
        generateStmt(node->children[2]);
        emit(std::make_shared<Instruction>(OpCode::JUMP, endLabel));
    } else {
        emit(std::make_shared<Instruction>(OpCode::LABEL, falseLabel, true));
    }
    
    emit(std::make_shared<Instruction>(OpCode::LABEL, endLabel, true));
}

void CodeGenerator::generateWhile(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;
    
    std::string testLabel = newLabel();
    std::string bodyLabel = newLabel();
    std::string endLabel = newLabel();
    
    emit(std::make_shared<Instruction>(OpCode::JUMP, testLabel));
    emit(std::make_shared<Instruction>(OpCode::LABEL, bodyLabel, true));
    
    setBlock(newBasicBlock());
    generateStmt(node->children[1]);
    
    emit(std::make_shared<Instruction>(OpCode::LABEL, testLabel, true));
    std::string cond = generateExpr(node->children[0]);
    emit(std::make_shared<Instruction>(OpCode::JNZ, cond, bodyLabel));
    
    emit(std::make_shared<Instruction>(OpCode::LABEL, endLabel, true));
}

void CodeGenerator::generateReturn(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) {
        emit(std::make_shared<Instruction>(OpCode::RETURN, "", "", ""));
    } else {
        std::string value = generateExpr(node->children[0]);
        emit(std::make_shared<Instruction>(OpCode::RETURN, value, "", ""));
    }
}

void CodeGenerator::generateAssignment(std::shared_ptr<ASTNode> node) {
    if (node->children.size() < 2) return;
    
    std::string left = generateExpr(node->children[0]);
    std::string right = generateExpr(node->children[1]);
    emit(std::make_shared<Instruction>(OpCode::ASSIGN, left, right, ""));
}

void CodeGenerator::generateCall(std::shared_ptr<ASTNode> node) {
    std::string funcName = node->value;
    
    for (auto& child : node->children) {
        std::string arg = generateExpr(child);
        emit(std::make_shared<Instruction>(OpCode::PARAM, arg, "", ""));
    }
    
    std::string result = newTemp();
    emit(std::make_shared<Instruction>(OpCode::CALL, result, funcName, ""));
}

std::string CodeGenerator::generateArrayAccess(std::shared_ptr<ASTNode> node) {
    std::string arrayName = node->value;
    std::string result = newTemp();
    
    for (auto& child : node->children) {
        std::string index = generateExpr(child);
    }
    
    emit(std::make_shared<Instruction>(OpCode::LOAD, result, arrayName, ""));
    return result;
}

void CodeGenerator::generateDecl(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) return;
    
    auto varNode = node->children[0];
    if (varNode->children.size() > 1) {
        std::string initValue = generateExpr(varNode->children[1]);
        std::string varName = varNode->children[0]->value;
        emit(std::make_shared<Instruction>(OpCode::ASSIGN, varName, initValue, ""));
    }
}

void CodeGenerator::generateFunction(std::shared_ptr<ASTNode> node) {
    if (node->children.empty()) return;
    
    std::string funcName = node->children[0]->value;
    std::string typeName = node->value;
    
    currentFunction = std::make_shared<Function>(funcName);
    currentFunction->returnType = typeName;
    functions[funcName] = currentFunction;
    
    auto entryBlock = newBasicBlock(funcName);
    currentFunction->entryBlock = entryBlock;
    setBlock(entryBlock);
    
    if (node->children.size() > 1) {
        for (size_t i = 1; i < node->children.size() - 1; i++) {
            auto paramNode = node->children[i];
            if (paramNode->type == NODE_IDENTIFIER) {
                currentFunction->params.push_back(paramNode->children[0]->value);
            }
        }
    }
    
    auto bodyNode = node->children.back();
    generateStmt(bodyNode);
    
    currentFunction = nullptr;
}

void CodeGenerator::generate(std::shared_ptr<ASTNode> root) {
    if (!root || root->type != NODE_COMP_UNIT) return;
    
    for (auto& child : root->children) {
        if (child->type == NODE_FUNC_DEF) {
            generateFunction(child);
        }
    }
}

std::string CodeGenerator::getGeneratedCode() const {
    std::ostringstream oss;
    
    for (const auto& pair : functions) {
        oss << pair.second->toString() << std::endl;
    }
    
    return oss.str();
}

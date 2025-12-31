#include "optimizer/optimizer.h"
#include <sstream>
#include <algorithm>
#include <cmath>

Optimizer::Optimizer() 
    : constantFoldings(0), deadCodeEliminations(0) {
}

Optimizer::~Optimizer() {
}

void Optimizer::optimize(std::map<std::string, std::shared_ptr<Function>>& functions) {
    for (auto& pair : functions) {
        optimizeFunction(pair.second);
    }
}

void Optimizer::optimizeFunction(std::shared_ptr<Function> func) {
    bool changed = true;
    int iterations = 0;
    
    while (changed && iterations < 10) {
        changed = false;
        iterations++;
        
        constantPropagation.clear();
        liveVariables.clear();
        
        std::shared_ptr<BasicBlock> block = func->entryBlock;
        std::set<std::shared_ptr<BasicBlock>> visited;
        std::vector<std::shared_ptr<BasicBlock>> queue;
        
        if (block) {
            queue.push_back(block);
            while (!queue.empty()) {
                block = queue.back();
                queue.pop_back();
                
                if (visited.find(block) != visited.end()) continue;
                visited.insert(block);
                
                if (constantFoldBlock(block)) {
                    changed = true;
                }
                
                if (eliminateDeadCode(block)) {
                    changed = true;
                }
                
                optimizeBlock(block);
                
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
        
        if (simplifyControlFlow(func)) {
            changed = true;
        }
    }
}

void Optimizer::optimizeBlock(std::shared_ptr<BasicBlock> block) {
    std::vector<std::shared_ptr<Instruction>> newInstructions;
    
    for (auto& inst : block->instructions) {
        if (inst->op == OpCode::ASSIGN && !inst->result.empty()) {
            constantPropagation[inst->result] = inst->arg1;
        }
        newInstructions.push_back(inst);
    }
    
    block->instructions = newInstructions;
}

bool Optimizer::constantFoldBlock(std::shared_ptr<BasicBlock> block) {
    bool changed = false;
    
    for (auto& inst : block->instructions) {
        if (inst->op == OpCode::ADD || inst->op == OpCode::SUB || 
            inst->op == OpCode::MUL || inst->op == OpCode::DIV ||
            inst->op == OpCode::MOD) {
            
            std::string left = inst->arg1;
            std::string right = inst->arg2;
            
            auto it1 = constantPropagation.find(left);
            if (it1 != constantPropagation.end()) left = it1->second;
            
            auto it2 = constantPropagation.find(right);
            if (it2 != constantPropagation.end()) right = it2->second;
            
            std::string result;
            if (evaluateBinaryOp(inst->op, left, right, result)) {
                inst->op = OpCode::ASSIGN;
                inst->arg1 = result;
                inst->arg2 = "";
                constantFoldings++;
                changed = true;
            }
        }
        
        else if (inst->op == OpCode::NEG || inst->op == OpCode::NOT) {
            std::string operand = inst->arg1;
            
            auto it = constantPropagation.find(operand);
            if (it != constantPropagation.end()) operand = it->second;
            
            std::string result;
            if (evaluateUnaryOp(inst->op, operand, result)) {
                inst->op = OpCode::ASSIGN;
                inst->arg1 = result;
                inst->arg2 = "";
                constantFoldings++;
                changed = true;
            }
        }
    }
    
    return changed;
}

bool Optimizer::eliminateDeadCode(std::shared_ptr<BasicBlock> block) {
    std::vector<std::shared_ptr<Instruction>> newInstructions;
    bool changed = false;
    
    liveVariables.clear();
    
    for (auto it = block->instructions.rbegin(); it != block->instructions.rend(); ++it) {
        auto inst = *it;
        
        if (inst->op == OpCode::ASSIGN && !inst->result.empty()) {
            if (liveVariables.find(inst->result) == liveVariables.end()) {
                deadCodeEliminations++;
                changed = true;
                continue;
            }
        }
        
        if (!inst->arg1.empty()) {
            auto it1 = constantPropagation.find(inst->arg1);
            if (it1 != constantPropagation.end()) {
                inst->arg1 = it1->second;
            }
            if (inst->arg1[0] != 't' && inst->arg1[0] != 'L') {
                liveVariables.insert(inst->arg1);
            }
        }
        
        if (!inst->arg2.empty()) {
            auto it2 = constantPropagation.find(inst->arg2);
            if (it2 != constantPropagation.end()) {
                inst->arg2 = it2->second;
            }
            if (inst->arg2[0] != 't' && inst->arg2[0] != 'L') {
                liveVariables.insert(inst->arg2);
            }
        }
        
        newInstructions.push_back(inst);
    }
    
    if (changed) {
        std::reverse(newInstructions.begin(), newInstructions.end());
        block->instructions = newInstructions;
    }
    
    return changed;
}

bool Optimizer::simplifyControlFlow(std::shared_ptr<Function> func) {
    return false;
}

bool Optimizer::isConstant(const std::string& value) {
    if (value.empty()) return false;
    return isNumeric(value);
}

bool Optimizer::isNumeric(const std::string& value) {
    if (value.empty()) return false;
    
    size_t start = 0;
    if (value[0] == '-') start = 1;
    
    bool hasDot = false;
    for (size_t i = start; i < value.length(); i++) {
        if (value[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (!isdigit(value[i])) {
            return false;
        }
    }
    
    return true;
}

double Optimizer::toNumber(const std::string& value) {
    return std::stod(value);
}

std::string Optimizer::fromNumber(double value, bool isInt) {
    std::ostringstream oss;
    if (isInt) {
        oss << (int)value;
    } else {
        oss << value;
    }
    return oss.str();
}

bool Optimizer::evaluateBinaryOp(OpCode op, const std::string& left, const std::string& right, std::string& result) {
    if (!isNumeric(left) || !isNumeric(right)) {
        return false;
    }
    
    double leftVal = toNumber(left);
    double rightVal = toNumber(right);
    double resultVal = 0;
    bool isInt = (left.find('.') == std::string::npos) && (right.find('.') == std::string::npos);
    
    switch (op) {
        case OpCode::ADD:
            resultVal = leftVal + rightVal;
            break;
        case OpCode::SUB:
            resultVal = leftVal - rightVal;
            break;
        case OpCode::MUL:
            resultVal = leftVal * rightVal;
            break;
        case OpCode::DIV:
            if (rightVal == 0) return false;
            resultVal = leftVal / rightVal;
            break;
        case OpCode::MOD:
            if (!isInt || rightVal == 0) return false;
            resultVal = (int)leftVal % (int)rightVal;
            break;
        default:
            return false;
    }
    
    result = fromNumber(resultVal, isInt);
    return true;
}

bool Optimizer::evaluateUnaryOp(OpCode op, const std::string& operand, std::string& result) {
    if (!isNumeric(operand)) {
        return false;
    }
    
    double val = toNumber(operand);
    double resultVal = 0;
    bool isInt = (operand.find('.') == std::string::npos);
    
    switch (op) {
        case OpCode::NEG:
            resultVal = -val;
            break;
        case OpCode::NOT:
            resultVal = (val == 0) ? 1 : 0;
            break;
        default:
            return false;
    }
    
    result = fromNumber(resultVal, isInt);
    return true;
}

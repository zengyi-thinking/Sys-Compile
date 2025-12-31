#include "target/target_codegen.h"
#include <sstream>
#include <algorithm>
#include <set>
bool isNumeric(const std::string& value);

TargetCodeGenerator::TargetCodeGenerator(TargetArch a) 
    : arch(a), stackPointer(0) {
    assemblyCode.str("");
    assemblyCode.clear();
    
    assemblyCode << "    .intel_syntax noprefix" << std::endl;
    assemblyCode << "    .text" << std::endl;
    assemblyCode << "    .global _start" << std::endl;
    assemblyCode << std::endl;
}

TargetCodeGenerator::~TargetCodeGenerator() {
}

void TargetCodeGenerator::emitLine(const std::string& line) {
    assemblyCode << "    " << line << std::endl;
}

std::string TargetCodeGenerator::allocRegister() {
    static const char* registers[] = {"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"};
    for (int i = 0; i < 10; i++) {
        std::string reg = registers[i];
        bool used = false;
        for (auto& pair : registerAllocation) {
            if (pair.second == reg) {
                used = true;
                break;
            }
        }
        if (!used) return reg;
    }
    return "r12";
}

void TargetCodeGenerator::freeRegister(const std::string& reg) {
    for (auto it = registerAllocation.begin(); it != registerAllocation.end(); ) {
        if (it->second == reg) {
            it = registerAllocation.erase(it);
        } else {
            ++it;
        }
    }
}

std::string TargetCodeGenerator::getRegister(const std::string& var) {
    auto it = registerAllocation.find(var);
    if (it != registerAllocation.end()) {
        return it->second;
    }
    
    std::string reg = allocRegister();
    registerAllocation[var] = reg;
    return reg;
}

void TargetCodeGenerator::generatePrologue(const std::string& funcName) {
    assemblyCode << funcName << ":" << std::endl;
    emitLine("push rbp");
    emitLine("mov rbp, rsp");
    
    if (stackPointer > 0) {
        emitLine("sub rsp, " + std::to_string(stackPointer));
    }
}

void TargetCodeGenerator::generateEpilogue() {
    emitLine("mov rsp, rbp");
    emitLine("pop rbp");
    emitLine("ret");
}

std::string TargetCodeGenerator::opToX86Op(OpCode op) {
    switch (op) {
        case OpCode::ADD: return "add";
        case OpCode::SUB: return "sub";
        case OpCode::MUL: return "imul";
        case OpCode::DIV: return "idiv";
        case OpCode::MOD: return "imul";
        case OpCode::NEG: return "neg";
        case OpCode::AND: return "and";
        case OpCode::OR: return "or";
        case OpCode::NOT: return "not";
        case OpCode::EQ: return "sete";
        case OpCode::NE: return "setne";
        case OpCode::LT: return "setl";
        case OpCode::LE: return "setle";
        case OpCode::GT: return "setg";
        case OpCode::GE: return "setge";
        default: return "";
    }
}

void TargetCodeGenerator::generateX86Instruction(std::shared_ptr<Instruction> inst) {
    switch (inst->op) {
        case OpCode::ASSIGN:
            if (!inst->result.empty()) {
                std::string reg = getRegister(inst->result);
                if (isNumeric(inst->arg1)) {
                    emitLine("mov " + reg + ", " + inst->arg1);
                } else {
                    std::string srcReg = getRegister(inst->arg1);
                    emitLine("mov " + reg + ", " + srcReg);
                }
            }
            break;
            
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL: {
            std::string reg1 = getRegister(inst->arg1);
            std::string reg2 = getRegister(inst->arg2);
            std::string reg = getRegister(inst->result);
            emitLine("mov " + reg + ", " + reg1);
            emitLine(opToX86Op(inst->op) + " " + reg + ", " + reg2);
            break;
        }
        
        case OpCode::DIV: {
            std::string reg1 = getRegister(inst->arg1);
            std::string reg2 = getRegister(inst->arg2);
            std::string reg = getRegister(inst->result);
            emitLine("mov rax, " + reg1);
            emitLine("cdq");
            emitLine("idiv " + reg2);
            emitLine("mov " + reg + ", rax");
            break;
        }
        
        case OpCode::NEG: {
            std::string reg1 = getRegister(inst->arg1);
            std::string reg = getRegister(inst->result);
            emitLine("mov " + reg + ", " + reg1);
            emitLine("neg " + reg);
            break;
        }
        
        case OpCode::LABEL:
            if (!inst->label.empty()) {
                assemblyCode << inst->label << ":" << std::endl;
            }
            break;
            
        case OpCode::JUMP:
            if (!inst->arg1.empty()) {
                emitLine("jmp " + inst->arg1);
            }
            break;
            
        case OpCode::JZ: {
            std::string reg = getRegister(inst->arg1);
            emitLine("test " + reg + ", " + reg);
            emitLine("jz " + inst->arg2);
            break;
        }
        
        case OpCode::JNZ: {
            std::string reg = getRegister(inst->arg1);
            emitLine("test " + reg + ", " + reg);
            emitLine("jnz " + inst->arg2);
            break;
        }
        
        case OpCode::CALL:
            if (!inst->arg1.empty()) {
                emitLine("call " + inst->arg1);
                if (!inst->result.empty()) {
                    std::string reg = getRegister(inst->result);
                    emitLine("mov " + reg + ", rax");
                }
            }
            break;
            
        case OpCode::RETURN:
            if (!inst->result.empty()) {
                std::string reg = getRegister(inst->result);
                emitLine("mov rax, " + reg);
            }
            emitLine("jmp .L_" + inst->result + "_epilogue");
            break;
            
        default:
            break;
    }
}

void TargetCodeGenerator::generateRISCVInstruction(std::shared_ptr<Instruction> inst) {
    generateX86Instruction(inst);
}

void TargetCodeGenerator::generateInstruction(std::shared_ptr<Instruction> inst) {
    if (arch == TargetArch::X86_64) {
        generateX86Instruction(inst);
    } else {
        generateRISCVInstruction(inst);
    }
}

void TargetCodeGenerator::generate(std::map<std::string, std::shared_ptr<Function>>& functions) {
    assemblyCode << std::endl;
    assemblyCode << "_start:" << std::endl;
    assemblyCode << "    call main" << std::endl;
    assemblyCode << "    mov rdi, rax" << std::endl;
    assemblyCode << "    mov rax, 60" << std::endl;
    assemblyCode << "    syscall" << std::endl;
    assemblyCode << std::endl;
    
    for (auto& pair : functions) {
        std::string funcName = pair.first;
        auto func = pair.second;
        
        generatePrologue(funcName);
        
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
                
                for (auto& inst : block->instructions) {
                    generateInstruction(inst);
                }
                
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
        
        generateEpilogue();
        assemblyCode << std::endl;
    }
}

bool isNumeric(const std::string& value) {
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

#ifndef TARGET_CODEGEN_H
#define TARGET_CODEGEN_H

#include "../codegen/code_generator.h"
#include <string>
#include <map>
#include <memory>
#include <sstream>

enum class TargetArch {
    X86_64,
    RISCV64
};

class TargetCodeGenerator {
private:
    TargetArch arch;
    std::ostringstream assemblyCode;
    std::map<std::string, int> localVarOffsets;
    int stackPointer;
    std::map<std::string, std::string> registerAllocation;
    
    std::string allocRegister();
    void freeRegister(const std::string& reg);
    std::string getRegister(const std::string& var);
    
    void generatePrologue(const std::string& funcName);
    void generateEpilogue();
    void generateInstruction(std::shared_ptr<Instruction> inst);
    
    std::string opToX86Op(OpCode op);
    std::string opToRISCVOps(OpCode op);
    
    void generateX86Instruction(std::shared_ptr<Instruction> inst);
    void generateRISCVInstruction(std::shared_ptr<Instruction> inst);
    
    void emitLine(const std::string& line);
    
public:
    TargetCodeGenerator(TargetArch a = TargetArch::X86_64);
    ~TargetCodeGenerator();
    
    void generate(std::map<std::string, std::shared_ptr<Function>>& functions);
    std::string getAssembly() const { return assemblyCode.str(); }
    
    void setArchitecture(TargetArch a) { arch = a; }
};

#endif

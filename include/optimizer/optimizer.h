#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "../codegen/code_generator.h"
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <string>

class Optimizer {
public:
    Optimizer();
    ~Optimizer();

    void optimize(std::map<std::string, std::shared_ptr<Function>>& functions);
    
    int getConstantFoldings() const { return constantFoldings; }
    int getDeadCodeEliminations() const { return deadCodeEliminations; }

private:
    int constantFoldings;
    int deadCodeEliminations;

    void optimizeFunction(std::shared_ptr<Function> func);
    void optimizeBlock(std::shared_ptr<BasicBlock> block);
    
    bool constantFoldBlock(std::shared_ptr<BasicBlock> block);
    bool eliminateDeadCode(std::shared_ptr<BasicBlock> block);
    bool simplifyControlFlow(std::shared_ptr<Function> func);
    
    bool isConstant(const std::string& value);
    bool isNumeric(const std::string& value);
    double toNumber(const std::string& value);
    std::string fromNumber(double value, bool isInt);
    
    std::map<std::string, std::string> constantPropagation;
    std::set<std::string> liveVariables;
    
    bool evaluateBinaryOp(OpCode op, const std::string& left, const std::string& right, std::string& result);
    bool evaluateUnaryOp(OpCode op, const std::string& operand, std::string& result);
};

#endif

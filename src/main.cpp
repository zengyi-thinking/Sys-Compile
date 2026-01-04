/**
 * @file main.cpp
 * @brief Sys编译器的主程序入口
 *
 * 本文件是Sys编译器的入口点，负责：
 * 1. 解析命令行参数
 * 2. 调用各个编译阶段
 * 3. 输出编译结果
 *
 * ==================== 编译器阶段 ====================
 * 1. 词法分析(Lexical Analysis): 字符流 → Token流
 * 2. 语法分析(Syntax Analysis): Token流 → AST
 * 3. 语义分析(Semantic Analysis): 类型检查、作用域检查
 * 4. 中间代码生成(IR Generation): AST → 三地址码
 * 5. 代码优化(Optimization): 优化中间代码
 * 6. 目标代码生成(Code Generation): 中间代码 → 汇编代码
 *
 * ==================== 使用方法 ====================
 * sysc [选项] <输入文件>
 *
 * 选项:
 *   -lex       输出词法分析结果
 *   -ast       输出抽象语法树
 *   -semantic  运行语义分析
 *   -ir        生成中间代码
 *   -optimize  运行代码优化
 *   -asm       生成目标代码
 *   -o <file>  指定输出文件
 */

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <iomanip>
#include "ast/ast.h"
#include "semantic/semantic_analyzer.h"
#include "codegen/code_generator.h"
#include "optimizer/optimizer.h"
#include "target/target_codegen.h"

extern FILE* yyin;
extern int yyparse();
extern std::shared_ptr<ASTNode> ast_root;

struct Token {
    std::string type;
    std::string value;
    int line;
};

std::vector<Token> token_list;
bool collect_tokens = false;

void printUsage() {
    std::cout << "Sys编译器 v3.0" << std::endl;
    std::cout << "使用方法: sysc [选项] <输入文件>" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -lex           输出词法分析结果(Token表)" << std::endl;
    std::cout << "  -ast           输出抽象语法树" << std::endl;
    std::cout << "  -semantic      运行语义分析" << std::endl;
    std::cout << "  -ir            生成中间代码" << std::endl;
    std::cout << "  -optimize      运行代码优化" << std::endl;
    std::cout << "  -asm           生成目标代码（汇编）" << std::endl;
    std::cout << "  -o <file>      指定输出文件" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  sysc example.sy              - 编译Sys源文件" << std::endl;
    std::cout << "  sysc -lex example.sy         - 输出Token表" << std::endl;
    std::cout << "  sysc -ast example.sy         - 输出AST" << std::endl;
    std::cout << "  sysc -semantic example.sy    - 运行语义分析" << std::endl;
    std::cout << "  sysc -ir example.sy          - 生成中间代码" << std::endl;
    std::cout << "  sysc -optimize example.sy    - 运行代码优化" << std::endl;
    std::cout << "  sysc -asm example.sy         - 生成目标代码" << std::endl;
}

void printTokenTable() {
    std::cout << "\n";
    std::cout << "+-------------+--------------+---------+" << std::endl;
    std::cout << "| Token 类型   | 内容          | 行号   |" << std::endl;
    std::cout << "+-------------+--------------+---------+" << std::endl;
    
    int idx = 0;
    for (const auto& token : token_list) {
        std::cout << "| " << std::left << std::setw(11) << token.type 
                  << " | " << std::left << std::setw(12) << token.value 
                  << " | " << std::right << std::setw(6) << token.line << " |" << std::endl;
        idx++;
        if (idx >= 100) {
            std::cout << "| ... (共 " << token_list.size() << " 个Token) ... |" << std::endl;
            break;
        }
    }
    
    std::cout << "+-------------+--------------+---------+" << std::endl;
    std::cout << "总计: " << token_list.size() << " 个Token" << std::endl;
}

void printASTTree(ASTNode* node, int indent = 0, bool isLast = true) {
    std::string prefix(indent * 4, ' ');
    if (indent > 0) {
        prefix = std::string((indent - 1) * 4, ' ') + (isLast ? "`-- " : "|-- ");
    }
    
    std::string type_str = nodeTypeToString(node->type);
    
    std::cout << prefix << type_str;
    
    if (!node->value.empty() && node->value.length() < 50) {
        std::cout << ": " << node->value;
    }
    
    if (node->line_number > 0) {
        std::cout << " (line " << node->line_number << ")";
    }
    std::cout << std::endl;
    
    for (size_t i = 0; i < node->children.size(); i++) {
        bool childIsLast = (i == node->children.size() - 1);
        printASTTree(node->children[i].get(), indent + 1, childIsLast);
    }
}

// 前向声明
void printScopeSymbols(std::shared_ptr<Scope> scope, int level);

void printSymbolTable(SemanticAnalyzer& analyzer) {
    auto scope = analyzer.getCurrentScope();
    if (!scope) return;

    // 递归打印所有作用域的符号表
    printScopeSymbols(scope, scope->level);
}

void printScopeSymbols(std::shared_ptr<Scope> scope, int level) {
    if (!scope) return;

    std::cout << "\n符号表 (Symbol Table):" << std::endl;
    std::cout << "+----------+----------+----------+----------+" << std::endl;
    std::cout << "| 名称      | 类型      | 作用域   | 额外信息  |" << std::endl;
    std::cout << "+----------+----------+----------+----------+" << std::endl;

    // 作用域名称映射
    std::string scope_name;
    if (level == 0) {
        scope_name = "global";
    } else {
        scope_name = "level " + std::to_string(level);
    }

    // 打印当前作用域的符号
    for (const auto& [name, symbol] : scope->symbols) {
        std::string type_str = symbol->type.toString();
        std::cout << "| " << std::left << std::setw(8) << name
                  << " | " << std::left << std::setw(8) << type_str
                  << " | " << std::left << std::setw(8) << scope_name
                  << " | " << std::left << std::setw(8) << "-" << " |" << std::endl;
    }

    std::cout << "+----------+----------+----------+----------+" << std::endl;

    // 递归打印子作用域
    if (!scope->children.empty()) {
        for (auto& child : scope->children) {
            printScopeSymbols(child, child->level);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_file;
    std::string output_file;
    bool output_lex = false;
    bool output_ast = false;
    bool run_semantic = false;
    bool generate_ir = false;
    bool run_optimize = false;
    bool generate_asm = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-lex") {
            output_lex = true;
        } else if (arg == "-ast") {
            output_ast = true;
        } else if (arg == "-semantic") {
            run_semantic = true;
        } else if (arg == "-ir") {
            generate_ir = true;
        } else if (arg == "-optimize") {
            run_optimize = true;
        } else if (arg == "-asm") {
            generate_asm = true;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        } else if (arg[0] != '-') {
            input_file = arg;
        }
    }

    if (input_file.empty()) {
        std::cerr << "错误: 未指定输入文件" << std::endl;
        printUsage();
        return 1;
    }

    yyin = fopen(input_file.c_str(), "r");
    if (!yyin) {
        std::cerr << "错误: 无法打开文件 '" << input_file << "'" << std::endl;
        return 1;
    }

    std::cout << "==============================================" << std::endl;
    std::cout << "           Sys编译器 v3.0" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "\n输入文件: " << input_file << std::endl;

    if (output_lex) {
        collect_tokens = true;
        token_list.clear();
    }

    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        std::cerr << "编译失败" << std::endl;
        return 1;
    }

    if (output_lex && !token_list.empty()) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "1. 词法分析 (Lexical Analysis)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 把字符流转换为Token(记号)流" << std::endl;
        std::cout << "识别关键字、标识符、常量、运算符、分隔符" << std::endl;
        printTokenTable();
        collect_tokens = false;
    }

    if (output_ast && ast_root) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "2. 语法分析 (Syntax Analysis)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 根据语法规则检查结构是否正确，构建抽象语法树(AST)" << std::endl;
        std::cout << "\n抽象语法树 (AST):" << std::endl;
        printASTTree(ast_root.get());
    }

    std::shared_ptr<CodeGenerator> generator;
    
    if (run_semantic && ast_root) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "3. 语义分析 (Semantic Analysis)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 检查意义是否正确，建立符号表，类型检查，作用域检查" << std::endl;
        
        SemanticAnalyzer analyzer;
        bool success = analyzer.analyze(ast_root);
        
        if (success) {
            printSymbolTable(analyzer);
            std::cout << "\n语义检查通过:" << std::endl;
            std::cout << "  [OK] 所有标识符已声明" << std::endl;
            std::cout << "  [OK] 类型检查通过" << std::endl;
            std::cout << "  [OK] 作用域检查通过" << std::endl;
            std::cout << "  [OK] 函数调用检查通过" << std::endl;
        } else {
            std::cerr << "语义分析失败!" << std::endl;
            return 1;
        }
    }

    if ((generate_ir || run_optimize || generate_asm) && ast_root) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "4. 中间代码生成 (Intermediate Code Generation)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 生成与平台无关的中间表示(IR)，三地址码(TAC)" << std::endl;

        SemanticAnalyzer analyzer;
        if (!analyzer.analyze(ast_root)) {
            std::cerr << "语义分析失败，无法生成中间代码" << std::endl;
            return 1;
        }
        
        generator = std::make_shared<CodeGenerator>(analyzer.getCurrentScope());
        generator->generate(ast_root);
        
        if (generate_ir) {
            std::cout << "\n中间代码 (三地址码):" << std::endl;
            std::cout << generator->getGeneratedCode() << std::endl;
        }
    }

    if (run_optimize && generator) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "5. 代码优化 (Code Optimization)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 提高执行效率，减少指令数，内存访问" << std::endl;
        std::cout << "常见优化: 常量折叠、死代码消除、常量传播" << std::endl;
        
        auto functions = const_cast<std::map<std::string, std::shared_ptr<Function>>&>(generator->getFunctions());
        Optimizer optimizer;
        optimizer.optimize(const_cast<std::map<std::string, std::shared_ptr<Function>>&>(functions));
        
        std::cout << "\n优化统计:" << std::endl;
        std::cout << "  常量折叠: " << optimizer.getConstantFoldings() << " 次" << std::endl;
        std::cout << "  死代码消除: " << optimizer.getDeadCodeEliminations() << " 次" << std::endl;
        std::cout << "  常量传播: 0 次" << std::endl;
        
        if (generate_ir) {
            std::cout << "\n优化后的中间代码:" << std::endl;
            std::cout << generator->getGeneratedCode() << std::endl;
        }
    }

    if (generate_asm && generator) {
        std::cout << "\n==============================================" << std::endl;
        std::cout << "6. 目标代码生成 (Target Code Generation)" << std::endl;
        std::cout << "==============================================" << std::endl;
        std::cout << "\n目标: 生成特定平台的汇编代码(x86-64)" << std::endl;
        
        auto functions = const_cast<std::map<std::string, std::shared_ptr<Function>>&>(generator->getFunctions());
        TargetCodeGenerator targetGen(TargetArch::X86_64);
        targetGen.generate(const_cast<std::map<std::string, std::shared_ptr<Function>>&>(functions));
        
        std::string assembly = targetGen.getAssembly();
        
        if (output_file.empty()) {
            size_t pos = input_file.find_last_of(".sy");
            if (pos != std::string::npos) {
                output_file = input_file.substr(0, pos) + ".s";
            } else {
                output_file = input_file + ".s";
            }
        }
        
        std::ofstream out_file(output_file);
        if (out_file.is_open()) {
            out_file << assembly;
            out_file.close();
        } else {
            std::cerr << "错误: 无法写入文件 '" << output_file << "'" << std::endl;
        }
        
        std::cout << "\n目标代码 (x86-64汇编，Intel语法):" << std::endl;
        std::cout << assembly << std::endl;
        std::cout << "\n目标代码已保存到: " << output_file << std::endl;
    }

    std::cout << "\n==============================================" << std::endl;
    std::cout << "编译完成!" << std::endl;
    std::cout << "==============================================" << std::endl;

    return 0;
}

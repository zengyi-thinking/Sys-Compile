#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "ast/ast.h"
#include "semantic/semantic_analyzer.h"
#include "codegen/code_generator.h"
#include "optimizer/optimizer.h"
#include "target/target_codegen.h"

// 外部函数声明
extern FILE* yyin;
extern int yyparse();
extern ASTNode* ast_root;

void printUsage() {
    std::cout << "Sys编译器 v3.0" << std::endl;
    std::cout << "使用方法: sysc [选项] <输入文件>" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -ast           输出抽象语法树" << std::endl;
    std::cout << "  -semantic      运行语义分析" << std::endl;
    std::cout << "  -ir            生成中间代码" << std::endl;
    std::cout << "  -optimize      运行代码优化" << std::endl;
    std::cout << "  -asm           生成目标代码（汇编）" << std::endl;
    std::cout << "  -o <file>      指定输出文件" << std::endl;
    std::cout <<  std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  sysc example.sy              - 编译Sys源文件" << std::endl;
    std::cout << "  sysc -ast example.sy         - 输出AST" << std::endl;
    std::cout << "  sysc -semantic example.sy    - 运行语义分析" << std::endl;
    std::cout << "  sysc -ir example.sy          - 生成中间代码" << std::endl;
    std::cout << "  sysc -optimize example.sy    - 运行代码优化" << std::endl;
    std::cout << "  sysc -asm example.sy         - 生成目标代码" << std::endl;
    std::cout << "  sysc -o output.s example.sy  - 指定输出文件" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_file;
    std::string output_file;
    bool output_ast = false;
    bool run_semantic = false;
    bool generate_ir = false;
    bool run_optimize = false;
    bool generate_asm = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-ast") {
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

    std::cout << "编译中: " << input_file << std::endl;

    int parse_result = yyparse();
    fclose(yyin);

    if (parse_result != 0) {
        std::cerr << "编译失败" << std::endl;
        return 1;
    }

    std::cout << "编译成功!" << std::endl;

    if (output_ast && ast_root) {
        std::cout << "\n抽象语法树:" << std::endl;
        ast_root->print();
    }

    if (run_semantic && ast_root) {
        std::cout << "\n语义分析..." << std::endl;
        SemanticAnalyzer analyzer;
        bool success = analyzer.analyze(std::shared_ptr<ASTNode>(ast_root, [](ASTNode*){}));
        
        if (success) {
            std::cout << "语义分析通过!" << std::endl;
        } else {
            std::cerr << "语义分析失败!" << std::endl;
            return 1;
        }
    }

    std::shared_ptr<CodeGenerator> generator;
    if ((generate_ir || run_optimize || generate_asm) && ast_root) {
        std::cout << "\n生成中间代码..." << std::endl;
        
        SemanticAnalyzer analyzer;
        if (!analyzer.analyze(std::shared_ptr<ASTNode>(ast_root, [](ASTNode*){}))) {
            std::cerr << "语义分析失败，无法生成中间代码" << std::endl;
            return 1;
        }
        
        generator = std::make_shared<CodeGenerator>(analyzer.getCurrentScope());
        generator->generate(std::shared_ptr<ASTNode>(ast_root, [](ASTNode*){}));
        
        if (generate_ir) {
            std::cout << "\n中间代码:" << std::endl;
            std::cout << generator->getGeneratedCode() << std::endl;
            std::cout << "中间代码生成完成!" << std::endl;
        } else {
            std::cout << "中间代码生成完成!" << std::endl;
        }
    }

    if (run_optimize && generator) {
        std::cout << "\n运行代码优化..." << std::endl;
        
        auto functions = const_cast<std::map<std::string, std::shared_ptr<Function>>&>(generator->getFunctions());
        Optimizer optimizer;
        optimizer.optimize(const_cast<std::map<std::string, std::shared_ptr<Function>>&>(functions));
        
        std::cout << "代码优化完成!" << std::endl;
        std::cout << "  常量折叠: " << optimizer.getConstantFoldings() << " 次" << std::endl;
        std::cout << "  死代码消除: " << optimizer.getDeadCodeEliminations() << " 次" << std::endl;
        
        if (generate_ir) {
            std::cout << "\n优化后的中间代码:" << std::endl;
            std::cout << generator->getGeneratedCode() << std::endl;
        }
    }

    if (generate_asm && generator) {
        std::cout << "\n生成目标代码..." << std::endl;
        
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
            std::cout << "目标代码已生成到: " << output_file << std::endl;
        } else {
            std::cerr << "错误: 无法写入文件 '" << output_file << "'" << std::endl;
            std::cout << "\n目标代码:" << std::endl;
            std::cout << assembly << std::endl;
        }
    }

    return 0;
}

#include <iostream>
#include <fstream>
#include <string>
#include "ast/ast.h"

// 外部函数声明
extern FILE* yyin;
extern int yyparse();
extern ASTNode* ast_root;

void printUsage() {
    std::cout << "Sys编译器 v1.0" << std::endl;
    std::cout << "使用方法: sysc [选项] <输入文件>" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -ast    输出抽象语法树" << std::endl;
    std::cout <<  std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  sysc example.sy              - 编译Sys源文件" << std::endl;
    std::cout << "  sysc -ast example.sy         - 输出AST" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string input_file;
    bool output_ast = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-ast") {
            output_ast = true;
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

    return 0;
}

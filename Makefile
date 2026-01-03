# 编译器
COMPILER = g++
# Flex和Bison - 使用Windows路径
FLEX = C:/GnuWin32/bin/flex.exe
BISON = C:/GnuWin32/bin/bison.exe
M4 = C:/GnuWin32/bin/m4.exe

# 设置M4路径
export M4 := C:/GnuWin32/bin/m4.exe

# 编译选项
CXXFLAGS = -std=c++17 -Wall -Wextra -Iinclude -g
LDFLAGS = 

# 源文件
LEXER_SRC = src/lexer/lexer.l
PARSER_SRC = src/parser/parser.y

# 生成的文件
LEXER_OUT = build/lexer.yy.c
PARSER_OUT = build/parser.tab.c
PARSER_HEADER = build/parser.tab.h

# 目标可执行文件
TARGET = build/sysc.exe

# 其他源文件
SRCS = src/main.cpp src/ast/ast.cpp src/semantic/semantic_analyzer.cpp src/codegen/code_generator.cpp src/optimizer/optimizer.cpp src/target/target_codegen.cpp
OBJS = build/main.o build/ast.o build/semantic_analyzer.o build/code_generator.o build/optimizer.o build/target_codegen.o

.PHONY: all clean test help

all: $(TARGET)

# 创建build目录
$(shell mkdir -p build 2>/dev/null || true)

# 构建词法分析器
$(LEXER_OUT): $(LEXER_SRC) $(PARSER_HEADER)
	@echo "生成词法分析器..."
	$(FLEX) $(LEXER_SRC)
	@if [ -f lex.yy.c ]; then mv lex.yy.c $(LEXER_OUT); fi

# 构建语法分析器
$(PARSER_OUT) $(PARSER_HEADER): $(PARSER_SRC)
	@echo "生成语法分析器..."
	set M4=C:/GnuWin32/bin/m4.exe && $(BISON) -d -o $(PARSER_OUT) $(PARSER_SRC)

# 编译C++源文件
build/main.o: src/main.cpp $(PARSER_HEADER)
	@echo "编译 main.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

build/ast.o: src/ast/ast.cpp $(PARSER_HEADER)
	@echo "编译 ast.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

build/semantic_analyzer.o: src/semantic/semantic_analyzer.cpp $(PARSER_HEADER)
	@echo "编译 semantic_analyzer.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

build/code_generator.o: src/codegen/code_generator.cpp $(PARSER_HEADER)
	@echo "编译 code_generator.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

build/optimizer.o: src/optimizer/optimizer.cpp $(PARSER_HEADER)
	@echo "编译 optimizer.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

build/target_codegen.o: src/target/target_codegen.cpp $(PARSER_HEADER)
	@echo "编译 target_codegen.cpp..."
	$(COMPILER) $(CXXFLAGS) -c $< -o $@

# 构建lexer.yy.o
build/lexer.yy.o: $(LEXER_OUT) $(PARSER_HEADER)
	@echo "编译词法分析器..."
	$(COMPILER) $(CXXFLAGS) -c $(LEXER_OUT) -o $@

# 构建parser.tab.o
build/parser.tab.o: $(PARSER_OUT) $(PARSER_HEADER)
	@echo "编译语法分析器..."
	$(COMPILER) $(CXXFLAGS) -c $(PARSER_OUT) -o $@

# 链接生成可执行文件
$(TARGET): build/lexer.yy.o build/parser.tab.o $(OBJS)
	@echo "链接编译器..."
	$(COMPILER) $(CXXFLAGS) $^ -o $(TARGET)
	@echo "构建完成: $(TARGET)"

# 测试
test: $(TARGET)
	@echo "运行测试..."
	./$(TARGET) examples/test.sy

# 清理
clean:
	@echo "清理构建文件..."
	-cd build && del /Q *.* 2>nul || true
	@echo "清理完成"

# 帮助信息
help:
	@echo "Sys编译器 - 使用说明"
	@echo ""
	@echo "可用命令:"
	@echo "  make          - 构建编译器"
	@echo "  make test     - 运行测试"
	@echo "  make clean    - 清理构建文件"
	@echo "  make help     - 显示此帮助信息"
	@echo ""
	@echo "使用示例:"
	@echo "  build\\sysc.exe examples\\test.sy          - 编译Sys源文件"
	@echo "  build\\sysc.exe -ast examples\\test.sy     - 输出AST"
	@echo "  build\\sysc.exe -semantic examples\\test.sy  - 运行语义分析"
	@echo "  build\\sysc.exe -ir examples\\test.sy       - 生成中间代码"
	@echo "  build\\sysc.exe -optimize examples\\test.sy - 运行代码优化"
	@echo "  build\\sysc.exe -asm examples\\test.sy      - 生成目标代码"

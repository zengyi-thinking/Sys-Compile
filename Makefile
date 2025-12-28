# 编译器
COMPILER = g++
# Flex和Bison
FLEX = flex
BISON = bison

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
TARGET = build/sysc

# 其他源文件
SRCS = $(wildcard src/**/*.cpp src/*.cpp)
OBJS = $(patsubst src/%.cpp,build/%.o,$(SRCS))

.PHONY: all clean test help

all: $(TARGET)

# 构建词法分析器
$(LEXER_OUT): $(LEXER_SRC) $(PARSER_HEADER)
	@echo "生成词法分析器..."
	@mkdir -p build
	$(FLEX) -o $(LEXER_OUT) $(LEXER_SRC)

# 构建语法分析器
$(PARSER_OUT) $(PARSER_HEADER): $(PARSER_SRC)
	@echo "生成语法分析器..."
	@mkdir -p build
	$(BISON) -d -o $(PARSER_OUT) $(PARSER_SRC)

# 编译C++源文件
build/%.o: src/%.cpp $(PARSER_HEADER)
	@echo "编译 $<..."
	@mkdir -p $(@D)
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
$(TARGET): build/lexer.yy.o build/parser.tab.o $(filter-out build/lexer.yy.o build/parser.tab.o,$(OBJS))
	@echo "链接编译器..."
	$(COMPILER) $(CXXFLAGS) $^ -o $(TARGET)
	@echo "构建完成: $(TARGET)"

# 测试
test: $(TARGET)
	@echo "运行测试..."
	@for file in examples/*.sy; do \
		if [ -f "$$file" ]; then \
			echo "测试: $$file"; \
			./$(TARGET) "$$file"; \
		fi; \
	done

# 清理
clean:
	@echo "清理构建文件..."
	rm -rf build/*
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
	@echo "  ./build/sysc examples/test.sy    - 编译Sys源文件"
	@echo "  ./build/sysc -ast examples/test.sy  - 输出AST"

# Sys 编译器 - 源代码说明

本目录包含 Sys 编译器的所有源代码实现。Sys 编译器是一个用 C++ 编写的教学型编译器，支持将 Sys 语言（C 的子集）编译为 x86-64 和 RISC-V 汇编代码。

## 目录结构

```
src/
├── ast/                 # 抽象语法树(AST)模块
│   └── ast.cpp          # AST实现
├── lexer/               # 词法分析器模块
│   └── lexer.l          # Flex词法规则定义
├── parser/              # 语法分析器模块
│   └── parser.y         # Bison语法规则定义
├── semantic/            # 语义分析器模块
│   └── semantic_analyzer.cpp
├── codegen/             # 中间代码生成器模块
│   └── code_generator.cpp
├── optimizer/           # 代码优化器模块
│   └── optimizer.cpp
├── target/              # 目标代码生成器模块
│   └── target_codegen.cpp
└── main.cpp             # 编译器主程序入口

include/
├── ast/                 # AST头文件
├── semantic/            # 语义分析器头文件
├── codegen/             # 中间代码生成器头文件
├── optimizer/           # 优化器头文件
└── target/              # 目标代码生成器头文件
```

## 模块说明

### 1. AST 模块 (`ast/`)

**文件**: `include/ast/ast.h`, `src/ast/ast.cpp`

**作用**: 定义和实现抽象语法树(AST)的数据结构。

**关键类**:
- `NodeType`: AST节点类型枚举
- `TypeKind`: 语义类型枚举
- `ASTNode`: AST节点类
- `Type`: 类型信息类

### 2. 词法分析器 (`lexer/`)

**文件**: `src/lexer/lexer.l`

**作用**: 将源代码字符流转换为Token流。

**工具**: Flex (Fast Lexical Analyzer Generator)

**识别内容**:
- 关键字: `int`, `float`, `void`, `if`, `while`, `return`, 等
- 运算符: `+`, `-`, `*`, `/`, `%`, `&&`, `||`, `==`, `!=`, 等
- 分隔符: `;`, `,`, `(`, `)`, `[`, `]`, `{`, `}`
- 字面量: 整数、浮点数、标识符

### 3. 语法分析器 (`parser/`)

**文件**: `src/parser/parser.y`

**作用**: 根据语法规则分析Token流，构建抽象语法树。

**工具**: Bison (GNU Yacc)

**生成内容**:
- `parser.tab.h`: Token定义和parser声明
- `parser.tab.c`: Parser实现代码

### 4. 语义分析器 (`semantic/`)

**文件**: `include/semantic/semantic_analyzer.h`, `src/semantic/semantic_analyzer.cpp`

**作用**: 对AST进行语义检查，确保程序在语义上是正确的。

**检查内容**:
- 变量是否先声明后使用
- 类型是否匹配
- 函数调用参数个数和类型是否正确
- return语句的返回值类型是否一致
- 变量/函数是否重复定义

**关键类**:
- `SemanticType`: 语义类型信息
- `Symbol`: 符号表条目
- `Scope`: 作用域管理
- `SemanticAnalyzer`: 语义分析器主类

### 5. 中间代码生成器 (`codegen/`)

**文件**: `include/codegen/code_generator.h`, `src/codegen/code_generator.cpp`

**作用**: 将AST转换为平台无关的中间代码（三地址码）。

**关键类**:
- `OpCode`: 中间代码操作码
- `Instruction`: 三地址码指令
- `BasicBlock`: 基本块
- `Function`: 函数的中间表示
- `CodeGenerator`: 代码生成器主类

**三地址码示例**:
```
源代码:   a = b + c * d
三地址码: t1 = c * d
          t2 = b + t1
          a = t2
```

### 6. 优化器 (`optimizer/`)

**文件**: `include/optimizer/optimizer.h`, `src/optimizer/optimizer.cpp`

**作用**: 对中间代码进行优化，提高执行效率。

**优化技术**:
- **常量折叠**: 编译时计算常量表达式 (如 `3 + 5` → `8`)
- **死代码消除**: 删除不会执行的代码
- **常量传播**: 用常量值替换变量
- **控制流简化**: 简化跳转指令

### 7. 目标代码生成器 (`target/`)

**文件**: `include/target/target_codegen.h`, `src/target/target_codegen.cpp`

**作用**: 将中间代码转换为特定平台的汇编代码。

**支持平台**:
- **x86-64**: Intel/AMD 64位架构 (使用Intel语法)
- **RISC-V**: 开源指令集架构 (64位)

**关键类**:
- `TargetArch`: 目标架构枚举
- `TargetCodeGenerator`: 目标代码生成器主类

### 8. 主程序 (`main.cpp`)

**文件**: `src/main.cpp`

**作用**: 编译器的入口点，协调各个编译阶段。

**功能**:
- 解析命令行参数
- 调用各个编译阶段
- 输出编译结果

## 编译流程

```
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│ 源代码  │ -> │ 词法分析 │ -> │ 语法分析 │ -> │   AST   │
│ .sy文件 │    │  Flex   │    │  Bison  │    │         │
└─────────┘    └─────────┘    └─────────┘    └─────────┘
                                                    │
                                                    v
┌─────────┐    ┌─────────┐    ┌─────────┐    ┌─────────┐
│ 汇编代码 │ <- │目标代码 │ <- │ 优化器  │ <- │语义分析 │
│  .s文件 │    │  生成   │    │         │    │         │
└─────────┘    └─────────┘    └─────────┘    └─────────┘
                                    │
                                    v
                              ┌─────────┐
                              │中间代码 │
                              │三地址码 │
                              └─────────┘
```

## 构建说明

### 依赖工具

- **C++ 编译器**: 支持 C++17 (gcc/clang/msvc)
- **Flex**: 词法分析器生成器
- **Bison**: 语法分析器生成器
- **CMake**: 构建系统 (可选)

### 构建步骤

```bash
# 1. 生成词法分析器
flex src/lexer/lexer.l -o build/lexer.yy.c

# 2. 生成语法分析器
bison -d src/parser/parser.y -o build/parser.tab.c

# 3. 编译所有源文件
g++ -std=c++17 -o sysc.exe \
    src/ast/ast.cpp \
    src/semantic/semantic_analyzer.cpp \
    src/codegen/code_generator.cpp \
    src/optimizer/optimizer.cpp \
    src/target/target_codegen.cpp \
    build/lexer.yy.o \
    build/parser.tab.o \
    src/main.cpp \
    -I include
```

## 使用示例

```bash
# 查看词法分析结果
sysc -lex examples/test.sy

# 查看抽象语法树
sysc -ast examples/test.sy

# 运行语义分析
sysc -semantic examples/test.sy

# 生成中间代码
sysc -ir examples/test.sy

# 运行优化
sysc -optimize examples/test.sy

# 生成汇编代码
sysc -asm examples/test.sy

# 完整编译
sysc examples/test.sy
```

## 开发者注意事项

1. **内存管理**: 使用 `std::shared_ptr` 管理 AST 节点，避免内存泄漏
2. **错误处理**: 使用 `std::cerr` 输出错误信息，设置适当的错误码
3. **代码风格**: 遵循 Doxygen 注释规范，保持代码可读性
4. **测试**: 使用 `examples/` 目录下的测试文件验证功能

## 扩展建议

- 添加更多数据类型支持 (char, string, 指针)
- 实现更高级的优化技术 (循环优化、内联展开)
- 支持更多目标平台 (ARM, MIPS)
- 添加调试信息生成 (DWARF 格式)

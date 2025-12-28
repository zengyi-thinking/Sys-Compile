# 快速开始指南

## 项目已创建完成！

项目结构如下：

```
Sys Compile/
├── include/
│   └── ast/
│       └── ast.h           # AST节点定义
├── src/
│   ├── lexer/
│   │   └── lexer.l         # Flex词法分析器
│   ├── parser/
│   │   └── parser.y        # Bison语法分析器
│   ├── ast/
│   │   └── ast.cpp         # AST实现
│   ├── semantic/           # 语义分析器（待实现）
│   ├── codegen/            # 代码生成器（待实现）
│   └── main.cpp            # 主程序入口
├── examples/
│   └── test.sy             # 测试示例
├── build/                  # 构建输出目录
├── Makefile                # 构建脚本
├── install.bat             # Windows安装检查脚本
├── README.md               # 项目说明
└── TODO.md                 # 开发进度

```

## 立即开始

### 第一步：安装依赖

在Windows上，推荐使用以下方法之一：

#### 方法A：MSYS2（推荐）
1. 下载并安装 MSYS2: https://www.msys2.org/
2. 打开 "MSYS2 MinGW 64-bit" 终端
3. 运行命令：
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make
```

#### 方法B：WinLibs + WinFlexBison
1. 下载 MinGW-w64: https://winlibs.com/
2. 下载 WinFlexBison: https://sourceforge.net/projects/winflexbison/
3. 将bin目录添加到系统PATH

### 第二步：检查环境

运行安装检查脚本：
```bash
./install.bat
```

或手动检查：
```bash
g++ --version
flex --version
bison --version
make --version
```

### 第三步：构建项目

```bash
make
```

如果成功，将在 build/ 目录生成 sysc 可执行文件

### 第四步：测试编译器

```bash
./build/sysc examples/test.sy
```

或查看AST：
```bash
./build/sysc -ast examples/test.sy
```

## 下一步开发

按照以下顺序逐步完善编译器：

1. **完善词法分析器** (src/lexer/lexer.l)
   - 添加更多Token类型
   - 处理注释
   - 改进错误处理

2. **完善语法分析器** (src/parser/parser.y)
   - 完善语法规则
   - 实现AST节点创建
   - 处理数组声明

3. **实现语义分析** (src/semantic/)
   - 设计符号表
   - 实现类型检查
   - 作用域管理

4. **生成中间代码** (src/codegen/)
   - 设计三地址码
   - 实现TAC生成

5. **生成目标代码**
   - 选择目标平台（RISC-V/x86）
   - 实现代码生成

## 常用命令

```bash
make          # 构建项目
make clean    # 清理构建文件
make test     # 运行测试
make help     # 显示帮助信息
```

## 遇到问题？

1. 找不到命令？检查PATH环境变量
2. 编译错误？确认Flex/Bison版本兼容
3. 运行错误？检查examples目录下的测试文件

详细说明请参考 README.md

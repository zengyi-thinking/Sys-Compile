# 快速开始 - 下一步操作

## 📦 当前状态

✅ **已完成的工作**：
- 项目结构搭建完成
- 词法分析器实现完成（支持注释）
- 语法分析器实现完成（含AST节点创建）
- AST系统实现完成
- 6个测试用例已创建
- 完整的文档已编写

⚠️ **需要立即完成**：
- 安装 flex 和 bison 工具
- 构建并测试编译器

## 🚀 立即开始的步骤

### 第1步：安装依赖工具

选择以下任一方法：

#### 方法A：WinFlexBison（推荐，最简单）
```bash
# 1. 下载 WinFlexBison
# https://github.com/lexxmark/winflexbison/releases
# 下载 winflexbison-2.5.25.zip

# 2. 解压到 D:\software\winflexbison

# 3. 添加到系统PATH
# 右键"此电脑" -> 属性 -> 高级系统设置 -> 环境变量
# 在系统变量"Path"中添加: D:\software\winflexbison

# 4. 创建别名（可选）
cd D:\software\winflexbison
copy win_flex.exe flex.exe
copy win_bison.exe bison.exe
```

#### 方法B：使用MSYS2
```bash
# 在 MSYS2 MinGW 64-bit 终端中运行：
pacman -S mingw-w64-x86_64-flex mingw-w64-x86_64-bison
```

### 第2步：验证安装

运行环境检查脚本：
```bash
check_env.bat
```

或者手动检查：
```bash
g++ --version
flex --version
bison --version
make --version
```

### 第3步：构建项目

```bash
make
```

如果成功，将看到：
```
生成词法分析器...
生成语法分析器...
编译 ...
链接编译器...
构建完成: build/sysc
```

### 第4步：测试编译器

运行测试：
```bash
make test
```

或单独测试某个文件：
```bash
./build/sysc examples/test_basic.sy
./build/sysc -ast examples/test_expr.sy
```

## 📁 项目文件说明

### 核心代码
- `src/lexer/lexer.l` - Flex词法分析器源文件
- `src/parser/parser.y` - Bison语法分析器源文件
- `src/ast/ast.cpp` - AST实现
- `include/ast/ast.h` - AST头文件
- `src/main.cpp` - 主程序入口

### 构建文件
- `Makefile` - 构建脚本

### 文档
- `README.md` - 项目说明
- `QUICKSTART.md` - 快速开始指南
- `INSTALL.md` - 详细安装指南
- `STATUS.md` - 项目进度报告
- `TODO.md` - 开发任务列表

### 测试用例
- `examples/test_basic.sy` - 基本变量和常量
- `examples/test_expr.sy` - 表达式计算
- `examples/test_if.sy` - if条件语句
- `examples/test_while.sy` - while循环
- `examples/test_func.sy` - 函数调用
- `examples/test_array.sy` - 数组操作

### 辅助脚本
- `check_env.bat` - 环境检查脚本
- `install_deps.bat` - 安装指导脚本

## 🎯 支持的语法特性

当前编译器可以解析以下语法：

### 数据类型
- ✅ int (32位有符号整数)
- ✅ float (32位浮点数)
- ✅ void (函数返回类型)
- ✅ 数组（声明和访问）

### 声明
- ✅ 变量声明
- ✅ 常量声明 (const)
- ✅ 数组声明

### 表达式
- ✅ 算术运算：+ - * / %
- ✅ 关系运算：< <= > >= == !=
- ✅ 逻辑运算：&& || !
- ✅ 一元运算：+ - !
- ✅ 函数调用

### 控制语句
- ✅ if / else
- ✅ while
- ✅ break
- ✅ continue
- ✅ return

### 其他
- ✅ 注释（// 和 /* */）
- ✅ 函数定义
- ✅ 函数参数
- ✅ 复合语句（代码块）

## 🔧 常用命令

```bash
# 构建项目
make

# 清理构建文件
make clean

# 运行测试
make test

# 显示帮助
make help

# 检查环境
check_env.bat
```

## ❓ 遇到问题？

### 找不到flex/bison命令
- 检查PATH环境变量
- 确认工具已正确安装
- 重启命令提示符

### 编译错误
- 确认flex/bison版本兼容（flex 2.5+, bison 3.0+）
- 检查g++版本（需要C++17支持）

### 运行错误
- 检查测试文件是否存在
- 查看错误信息了解具体问题

## 📚 参考文档

详细的说明请查阅：
- `INSTALL.md` - 详细的安装指南
- `README.md` - 完整的项目说明
- `STATUS.md` - 项目进度和已知问题

## 🎉 成功的标志

当所有步骤完成后，你应该能够：

1. ✅ 运行 `check_env.bat` 显示所有工具已安装
2. ✅ 运行 `make` 成功编译，生成 `build/sysc.exe`
3. ✅ 运行 `./build/sysc examples/test_basic.sy` 显示"编译成功"
4. ✅ 运行 `./build/sysc -ast examples/test_expr.sy` 显示AST结构

## 🚀 完成基础构建后的下一步

安装完依赖并成功构建后，可以继续开发：

1. **语义分析**：实现符号表和类型检查
2. **中间代码生成**：生成三地址码
3. **目标代码生成**：生成RISC-V或x86汇编

详细任务列表请查看 `TODO.md`

---

**现在就开始安装依赖，然后运行 `make` 构建项目吧！** 🎯

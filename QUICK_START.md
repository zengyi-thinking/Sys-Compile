# Sys编译器 - 快速开始

## ✅ 已完成的功能

### 1. 删除UI界面
- ✅ 移除了所有图形界面相关文件
- ✅ 项目专注于命令行编译器核心功能

### 2. 语义分析器（新）
- ✅ 类型系统（int, float, void, array）
- ✅ 符号表和作用域管理
- ✅ 语义检查（变量、函数、表达式、控制流）
- ✅ 详细的错误报告

### 3. 中间代码生成器（新）
- ✅ 三地址码生成
- ✅ 算术、关系、逻辑运算
- ✅ 控制流指令（跳转、标签）
- ✅ 函数调用和返回
- ✅ 基本块管理

## ⚠️ 首次使用需要安装依赖

**当前状态**：源代码已完成，但需要安装 Flex、Bison、GCC 才能编译

### 快速安装（MSYS2）

```bash
# 1. 安装 MSYS2
# 下载：https://www.msys2.org/
# 安装到 C:\msys64

# 2. 打开 MSYS2 MinGW 64-bit 终端，运行：
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make

# 3. 将 C:\msys64\mingw64\bin 添加到系统 PATH

# 4. 重启命令行，验证安装
g++ --version
flex --version
bison --version
```

### 依赖检查

运行检查脚本：
```bash
check_dependencies.bat
```

详细安装指南：参见 `INSTALLATION_GUIDE.md`

## 🚀 安装依赖后，开始使用

### 1. 编译项目
```bash
make clean
make
```

### 2. 运行测试
```bash
make test
```

### 3. 使用编译器

```bash
# 基本编译
build\sysc.exe examples\test.sy

# 输出抽象语法树
build\sysc.exe -ast examples\test.sy

# 运行语义分析
build\sysc.exe -semantic examples\test.sy

# 生成中间代码
build\sysc.exe -ir examples\test.sy

# 完整流程
build\sysc.exe -ast -semantic -ir examples\test.sy
```

## 📁 项目结构

```
Sys Compile/
├── include/
│   ├── ast/                    # 抽象语法树头文件
│   ├── semantic/               # 语义分析器头文件
│   └── codegen/                # 代码生成器头文件
├── src/
│   ├── lexer/                 # 词法分析器
│   ├── parser/                # 语法分析器
│   ├── semantic/               # 语义分析器实现
│   ├── codegen/                # 代码生成器实现
│   └── main.cpp                # 主程序
├── examples/                   # 测试用例
├── build/                      # 编译输出目录
├── Makefile                    # 构建脚本
├── check_dependencies.bat      # 依赖检查脚本
└── INSTALLATION_GUIDE.md       # 详细安装指南
```

## 🎯 编译器架构

```
源代码
  ↓
词法分析器 (Flex) ← 需要安装 Flex
  ↓
语法分析器 (Bison) ← 需要安装 Bison
  ↓
抽象语法树 (AST) ← ✅ 已完成
  ↓
语义分析器 ← ✅ 已完成
  ↓
中间代码生成器 ← ✅ 已完成
  ↓
目标代码生成器 (待实现)
```

## 📊 当前进度

- ✅ 词法分析器 (100%)
- ✅ 语法分析器 (100%)
- ✅ 抽象语法树 (100%)
- ✅ 语义分析器 (100%)
- ✅ 中间代码生成器 (100%)
- ⏳ 代码优化 (0%)
- ⏳ 目标代码生成 (0%)

## 🆘 帮助

### 问题：找不到 bison 命令
**解决**：安装 MSYS2 并将 `C:\msys64\mingw64\bin` 添加到 PATH

### 问题：编译失败
**解决**：运行 `check_dependencies.bat` 检查依赖

### 问题：PATH 设置后仍然无效
**解决**：重启命令行或计算机

## 📚 相关文档

- `INSTALLATION_GUIDE.md` - 详细安装指南
- `UPDATE_SEMANTIC_CODEGEN.md` - 语义分析和代码生成说明
- `PROJECT_SUMMARY.md` - 项目总结
- `README.md` - 原始文档

## 🎓 学习资源

- Flex 手册：https://westes.github.io/flex/manual/
- Bison 手册：https://www.gnu.org/software/bison/manual/
- SysY 语言定义：https://gitlab.eduxiji.net/nscscc/compiler2022/-/blob/master/SysY2022语言定义V1.pdf

---

**状态**：✅ 代码已完成，等待安装依赖后编译
**下一步**：按照 `INSTALLATION_GUIDE.md` 安装依赖，然后运行 `make`

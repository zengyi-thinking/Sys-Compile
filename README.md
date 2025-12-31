# Sys编译器项目

## 项目简介

这是一个用于Sys语言的编译器实现，支持基本的语法分析和AST生成。

## 功能特性

- 支持int和float类型
- 支持多维数组（按行优先存储）
- 支持常量声明（const）
- 支持类型隐式转换
- 支持函数定义和调用
- 支持控制语句（if, while, break, continue, return）

## 项目结构

```
.
├── include/           # 头文件
│   └── ast/          # AST定义
├── src/              # 源代码
│   ├── lexer/        # 词法分析器
│   ├── parser/       # 语法分析器
│   ├── semantic/     # 语义分析器
│   ├── codegen/      # 代码生成器
│   └── ast/          # AST实现
├── examples/         # 测试示例
├── build/            # 构建输出目录
└── Makefile          # 构建脚本
```

## 环境要求

- GCC或Clang编译器（支持C++17）
- Flex（词法分析器生成器）
- Bison（语法分析器生成器）
- Make工具

## 安装依赖

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make
```

### Windows (MinGW)
1. 从 [GnuWin32](http://gnuwin32.sourceforge.net/) 下载 Flex 和 Bison
2. 从 [MinGW](https://www.mingw-w64.org/) 下载 GCC
3. 将这些工具添加到系统PATH

### Windows (Cygwin)
在Cygwin安装器中选择以下包：
- gcc-core
- flex
- bison
- make

### Linux
```bash
sudo apt-get install flex bison g++ make
```

### macOS
```bash
brew install flex bison
```

## 构建项目

```bash
# 克隆项目后进入目录
cd "Sys Compile"

# 构建编译器
make

# 运行测试
make test

# 清理构建文件
make clean
```

## 使用方法

### 方式一：使用图形界面（推荐）

双击运行 `start_gui.bat` 启动图形界面：

```bash
start_gui.bat
```

图形界面功能：
- 📝 源代码编辑器
- 🔍 编译功能
- 🌳 AST可视化
- 📚 内置测试用例
- 💾 保存源代码

### 方式二：命令行使用

```bash
# 编译Sys源文件
./build/sysc examples/test.sy

# 输出AST
./build/sysc -ast examples/test.sy
```

## 测试用例

项目包含多个测试用例：

- `test_basic.sy` - 基本变量和常量
- `test_expr.sy` - 表达式计算
- `test_if.sy` - if条件语句
- `test_while.sy` - while循环
- `test_func.sy` - 函数调用
- `test_array.sy` - 数组操作

运行所有测试：
```bash
make test
```

## Sys语言示例

```c
int main() {
    int a = 10;
    float b = 3.14;
    int result = a + (int)b;
    
    if (result > 15) {
        return result;
    }
    
    return 0;
}
```

## 开发计划

- [x] 项目骨架搭建
- [x] 完善词法分析器（支持注释）
- [x] 完善语法分析器（AST节点创建）
- [x] 添加基础测试用例
- [ ] 实现语义分析
- [ ] 生成中间代码
- [ ] 生成目标代码

## 参考资料

- [Flex手册](https://westes.github.io/flex/manual/)
- [Bison手册](https://www.gnu.org/software/bison/manual/)
- [SysY2022语言定义](https://gitlab.eduxiji.net/nscscc/compiler2022/-/blob/master/SysY2022语言定义V1.pdf)

## 许可证

MIT License

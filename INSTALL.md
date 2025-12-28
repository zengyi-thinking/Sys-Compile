# Flex和Bison安装指南

## Windows系统

### 方法1：使用WinFlexBison（推荐，最简单）

WinFlexBison是Windows版本的Flex和Bison，不需要安装MSYS2或Cygwin。

1. **下载**
   - 访问：https://github.com/lexxmark/winflexbison/releases
   - 下载最新的 `winflexbison-2.5.25.zip`（或最新版本）

2. **解压**
   - 解压到：`D:\software\winflexbison`
   - 确保目录结构：
     ```
     D:\software\winflexbison\
     ├── win_flex.exe
     ├── win_bison.exe
     └── ...
     ```

3. **添加到PATH**
   - 打开"系统属性" -> "高级" -> "环境变量"
   - 在"系统变量"中找到"Path"，点击"编辑"
   - 添加新条目：`D:\software\winflexbison`

4. **创建别名**（可选）
   为了兼容性，可以创建`flex.exe`和`bison.exe`：
   ```cmd
   cd D:\software\winflexbison
   copy win_flex.exe flex.exe
   copy win_bison.exe bison.exe
   ```

5. **验证安装**
   打开新的命令提示符或PowerShell窗口，运行：
   ```cmd
   flex --version
   bison --version
   ```

### 方法2：使用MSYS2

你已经安装了MSYS2，只需安装flex和bison包：

1. **打开MSYS2 MinGW 64-bit终端**
   - 在开始菜单中找到"MSYS2 MinGW 64-bit"并打开

2. **更新包数据库**
   ```bash
   pacman -Syu
   ```

3. **安装flex和bison**
   ```bash
   pacman -S mingw-w64-x86_64-flex mingw-w64-x86_64-bison
   ```

4. **配置PATH**
   将 `D:\software\msys64\mingw64\bin` 添加到系统PATH

5. **验证安装**
   ```bash
   flex --version
   bison --version
   ```

### 方法3：使用Cygwin

1. 下载Cygwin安装程序：https://www.cygwin.com/

2. 运行安装程序，选择以下包：
   - `flex`
   - `bison`
   - `gcc-core`
   - `make`

3. 完成安装后，验证：
   ```bash
   flex --version
   bison --version
   ```

## Linux系统

```bash
# Ubuntu/Debian
sudo apt-get install flex bison

# Fedora/RHEL
sudo dnf install flex bison

# Arch Linux
sudo pacman -S flex bison
```

## macOS系统

```bash
# 使用Homebrew
brew install flex bison

# 验证安装
flex --version
bison --version
```

## 故障排除

### 问题1：找不到命令

**症状**：运行`flex`或`bison`时提示"命令不存在"

**解决方案**：
1. 确认工具已正确安装
2. 检查PATH环境变量是否包含工具的bin目录
3. 重启命令提示符/终端窗口

### 问题2：版本不兼容

**症状**：编译时出现语法错误

**解决方案**：
- Flex 2.5.x 或更高版本
- Bison 3.0.x 或更高版本

检查版本：
```bash
flex --version
bison --version
```

### 问题3：WinFlexBison路径问题

**症状**：使用WinFlexBison时编译失败

**解决方案**：
确保创建了`flex.exe`和`bison.exe`的副本：
```cmd
cd D:\software\winflexbison
copy win_flex.exe flex.exe
copy win_bison.exe bison.exe
```

## 验证安装

安装完成后，运行以下命令验证所有依赖：

```bash
# 检查编译器
g++ --version

# 检查Flex
flex --version

# 检查Bison
bison --version

# 检查Make
make --version
```

所有命令都应该显示版本信息，没有错误。

## 下一步

安装完成后，继续构建项目：

```bash
make
```

如果成功，将生成 `build/sysc` 可执行文件。

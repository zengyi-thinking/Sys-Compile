# 依赖安装和编译指南

## ⚠️ 当前问题

编译失败是因为系统中缺少 **Flex** 和 **Bison** 工具。

## 🔧 解决方案

### 方案一：使用 MSYS2（推荐）

1. **安装 MSYS2**
   - 下载：https://www.msys2.org/
   - 安装到默认路径：`C:\msys64`

2. **安装依赖包**
   打开 MSYS2 MinGW 64-bit 终端，运行：
   ```bash
   pacman -Syu
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make
   ```

3. **添加到 PATH**
   将 `C:\msys64\mingw64\bin` 添加到系统环境变量 PATH

4. **验证安装**
   打开新的命令行窗口，运行：
   ```bash
   where g++ flex bison make
   ```

### 方案二：手动下载安装

#### 1. 安装 GCC (MinGW-w64)
- 下载：https://www.mingw-w64.org/downloads/
- 或使用 TDM-GCC：https://jmeubank.github.io/tdm-gcc/
- 安装后添加 `bin` 目录到 PATH

#### 2. 安装 Flex
- 下载：http://gnuwin32.sourceforge.net/packages/flex.htm
- 解压后将 `bin` 目录添加到 PATH

#### 3. 安装 Bison
- 下载：http://gnuwin32.sourceforge.net/packages/bison.htm
- 解压后将 `bin` 目录添加到 PATH

#### 4. 安装 Make（可选）
- 下载：http://gnuwin32.sourceforge.net/packages/make.htm
- 解压后将 `bin` 目录添加到 PATH

## 📝 完整安装步骤（MSYS2）

### 1. 下载并安装 MSYS2
```powershell
# 以管理员身份运行 PowerShell
# 下载并运行 MSYS2 安装器
# 安装到 C:\msys64
```

### 2. 安装依赖包
打开 **MSYS2 MinGW 64-bit** 终端：
```bash
# 更新系统
pacman -Syu

# 安装编译工具链
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make
```

### 3. 配置环境变量
将以下路径添加到系统 PATH：
```
C:\msys64\mingw64\bin
```

**设置方法**：
1. 右键"此电脑" → 属性 → 高级系统设置
2. 环境变量 → 系统变量 → Path → 编辑
3. 新建 → 添加 `C:\msys64\mingw64\bin`
4. 确定 → 重启命令行

### 4. 验证安装
```bash
# 检查工具是否可用
g++ --version
flex --version
bison --version
make --version
```

## 🚀 编译项目

依赖安装完成后，在项目目录运行：

```bash
# 1. 清理旧的构建文件
make clean

# 2. 构建项目
make

# 3. 运行测试
make test
```

## 🔍 验证工具

运行检查脚本：
```bash
check_dependencies.bat
```

## ❓ 常见问题

### 1. 找不到 bison 命令
**原因**：bison 未安装或未添加到 PATH  
**解决**：按照上述步骤安装并配置环境变量

### 2. Windows 编码问题
**原因**：命令行显示乱码  
**解决**：在 cmd 中运行 `chcp 65001` 切换到 UTF-8

### 3. 权限问题
**原因**：需要管理员权限  
**解决**：以管理员身份运行命令行

### 4. 路径问题
**原因**：路径中包含空格或中文字符  
**解决**：确保项目路径只包含英文字符

## 📦 快速安装脚本

如果已有 Chocolatey：
```powershell
choco install mingw flex bison make
```

如果已有 Scoop：
```powershell
scoop install gcc flex bison make
```

## 🎯 下一步

依赖安装成功后，您就可以：
1. 构建编译器：`make`
2. 运行测试：`make test`
3. 使用编译器：`build\sysc.exe examples\test.sy`

## 📞 需要帮助？

如果遇到问题：
1. 检查每个工具是否可用：`where g++ flex bison make`
2. 运行依赖检查脚本：`check_dependencies.bat`
3. 查看详细日志：`make -v`

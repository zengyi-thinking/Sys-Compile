@echo off
REM Windows下安装flex和bison的指导脚本

echo ==========================================
echo Sys编译器 - 依赖安装指南
echo ==========================================
echo.

echo 检查已安装的工具...
echo.

where g++ >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] g++ 已安装
) else (
    echo [X] g++ 未安装
)

where flex >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] flex 已安装
) else (
    echo [X] flex 未安装
)

where bison >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] bison 已安装
) else (
    echo [X] bison 未安装
)

echo.
echo ==========================================
echo 安装flex和bison（两种方法）
echo ==========================================
echo.

echo 方法1: 使用WinFlexBison（推荐，纯Windows工具）
echo   1. 访问: https://github.com/lexxmark/winflexbison/releases
echo   2. 下载最新的 winflexbison.zip
echo   3. 解压到: D:\software\winflexbison
echo   4. 添加 D:\software\winflexbison 到系统PATH
echo.

echo 方法2: 使用MSYS2包管理器
echo   1. 打开 MSYS2 MinGW 64-bit 终端
echo   2. 运行命令:
echo      pacman -S mingw-w64-x86_64-flex mingw-w64-x86_64-bison
echo   3. 重启终端
echo.

echo 方法3: 使用Cygwin
echo   1. 运行 Cygwin setup-x86_64.exe
echo   2. 选择包: flex, bison
echo.

pause

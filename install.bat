@echo off
echo ==========================================
echo Sys编译器 - 开发环境快速安装指南
echo ==========================================
echo.

echo [1] 检测当前环境...
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

where make >nul 2>nul
if %errorlevel% equ 0 (
    echo [OK] make 已安装
) else (
    echo [X] make 未安装
)

echo.
echo ==========================================
echo 安装方法:
echo ==========================================
echo.
echo 方法1: 使用 MSYS2 (推荐)
echo   1. 从 https://www.msys2.org/ 下载并安装 MSYS2
echo   2. 打开 MSYS2 MinGW 64-bit 终端
echo   3. 运行: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-flex mingw-w64-x86_64-bison mingw-w64-x86_64-make
echo   4. 将 MSYS2\mingw64\bin 添加到系统PATH
echo.
echo 方法2: 使用 MinGW
echo   1. 从 https://winlibs.com/ 下载 MinGW-w64
echo   2. 从 https://sourceforge.net/projects/winflexbison/ 下载 win_flex_bison
echo   3. 将所有bin目录添加到系统PATH
echo.
echo 方法3: 使用 Cygwin
echo   1. 从 https://www.cygwin.com/ 下载 setup-x86_64.exe
echo   2. 安装时选择: gcc-core, flex, bison, make
echo.
echo ==========================================
echo 安装完成后:
echo ==========================================
echo   运行: make
echo   运行: make test
echo.
pause

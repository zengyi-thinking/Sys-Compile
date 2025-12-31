@echo off
echo ========================================
echo Sys编译器 - 依赖安装脚本
echo ========================================
echo.

echo 检查Flex...
where flex >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到Flex，请先安装
    echo.
    echo 推荐安装方法：
    echo 1. 下载 GnuWin32 的 Flex 包: http://gnuwin32.sourceforge.net/packages/flex.htm
    echo 2. 或使用 MSYS2: pacman -S mingw-w64-x86_64-flex
    echo 3. 安装后添加到系统PATH
    pause
    exit /b 1
) else (
    echo [√] Flex 已安装
)

echo.
echo 检查Bison...
where bison >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到Bison，请先安装
    echo.
    echo 推荐安装方法：
    echo 1. 下载 GnuWin32 的 Bison 包: http://gnuwin32.sourceforge.net/packages/bison.htm
    echo 2. 或使用 MSYS2: pacman -S mingw-w64-x86_64-bison
    echo 3. 安装后添加到系统PATH
    pause
    exit /b 1
) else (
    echo [√] Bison 已安装
)

echo.
echo 检查GCC...
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 未找到GCC，请先安装
    echo.
    echo 推荐安装方法：
    echo 1. 下载 MinGW-w64: https://www.mingw-w64.org/
    echo 2. 或使用 MSYS2: pacman -S mingw-w64-x86_64-gcc
    echo 3. 安装后添加到系统PATH
    pause
    exit /b 1
) else (
    echo [√] GCC 已安装
)

echo.
echo 检查Make...
where make >nul 2>&1
if %errorlevel% neq 0 (
    echo [警告] 未找到Make，但可以使用其他构建方法
    echo.
    echo 如果需要使用make：
    echo 1. 下载 GnuWin32 的 Make 包: http://gnuwin32.sourceforge.net/packages/make.htm
    echo 2. 或使用 MSYS2: pacman -S mingw-w64-x86_64-make
) else (
    echo [√] Make 已安装
)

echo.
echo ========================================
echo 所有依赖检查完成！
echo ========================================
echo.
echo 现在可以运行以下命令构建编译器：
echo   make
echo.
pause

@echo off
REM 临时设置MSYS2环境变量并启动编译器构建

echo ==========================================
echo 设置MSYS2环境变量
echo ==========================================
echo.

REM 设置MSYS2路径
set "MSYS2_PATH=D:\software\msys64"
set "PATH=%MSYS2_PATH%\usr\bin;%MSYS2_PATH%\mingw64\bin;%PATH%"

echo MSYS2路径已添加:
echo   %MSYS2_PATH%\usr\bin
echo   %MSYS2_PATH%\mingw64\bin
echo.

echo 验证安装:
flex --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] flex 已安装
    flex --version
) else (
    echo [X] flex 未找到
)
echo.

bison --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] bison 已安装
    bison --version
) else (
    echo [X] bison 未找到
)
echo.

g++ --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] g++ 已安装
    g++ --version 2>&1 | findstr "g++"
) else (
    echo [X] g++ 未找到
)
echo.

make --version 2>nul
if %errorlevel% equ 0 (
    echo [OK] make 已安装
    make --version 2>&1 | findstr "GNU Make"
) else (
    echo [X] make 未找到
)
echo.

echo ==========================================
echo 现在可以运行: make
echo ==========================================
echo.

REM 保持在当前shell中，以便用户可以直接运行make
cmd /k

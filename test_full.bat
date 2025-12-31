@echo off
echo ========================================
echo Sys编译器 - 完整流程测试
echo ========================================
echo.

if not exist "build\sysc.exe" (
    echo [错误] 编译器未构建，请先运行: make
    pause
    exit /b 1
)

echo [1/5] 测试基本编译...
build\sysc.exe examples\test_basic.sy
if %errorlevel% neq 0 (
    echo [失败] 基本编译测试失败
    pause
    exit /b 1
)
echo [通过] 基本编译测试成功
echo.

echo [2/5] 测试语义分析...
build\sysc.exe -semantic examples\test_basic.sy
if %errorlevel% neq 0 (
    echo [失败] 语义分析测试失败
    pause
    exit /b 1
)
echo [通过] 语义分析测试成功
echo.

echo [3/5] 测试中间代码生成...
build\sysc.exe -ir examples\test_func.sy > test_ir.txt
if %errorlevel% neq 0 (
    echo [失败] 中间代码生成测试失败
    pause
    exit /b 1
)
echo [通过] 中间代码生成测试成功
echo.

echo [4/5] 测试代码优化...
build\sysc.exe -optimize examples\test_expr.sy
if %errorlevel% neq 0 (
    echo [失败] 代码优化测试失败
    pause
    exit /b 1
)
echo [通过] 代码优化测试成功
echo.

echo [5/5] 测试目标代码生成...
build\sysc.exe -asm examples\test_basic.sy
if %errorlevel% neq 0 (
    echo [失败] 目标代码生成测试失败
    pause
    exit /b 1
)
if exist examples\test_basic.s (
    echo [通过] 目标代码生成成功: examples\test_basic.s
) else (
    echo [失败] 未生成目标代码文件
    pause
    exit /b 1
)
echo.

echo ========================================
echo 所有测试通过！
echo ========================================
echo.
echo 测试结果摘要:
echo   [√] 基本编译
echo   [√] 语义分析
echo   [√] 中间代码生成
echo   [√] 代码优化
echo   [√] 目标代码生成
echo.

pause

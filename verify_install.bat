@echo off
echo ==========================================
echo 验证编译器环境
echo ==========================================
echo.

set errors=0

echo [1] 检查 g++...
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    g++ --version 2>&1 | findstr /C:"g++" >nul
    if %errorlevel% equ 0 (
        echo [OK] g++ 已安装
        for /f "tokens=*" %%i in ('g++ --version 2^>^&1') do (
            echo     %%i
            goto :gpp_done
        )
        :gpp_done
    ) else (
        echo [X] g++ 检测失败
        set /a errors+=1
    )
) else (
    echo [X] g++ 未安装
    set /a errors+=1
)
echo.

echo [2] 检查 flex...
where flex >nul 2>nul
if %errorlevel% equ 0 (
    flex --version 2>&1 | findstr /C:"flex" >nul
    if %errorlevel% equ 0 (
        echo [OK] flex 已安装
        for /f "tokens=*" %%i in ('flex --version 2^>^&1') do (
            echo     %%i
            goto :flex_done
        )
        :flex_done
    ) else (
        echo [X] flex 检测失败
        set /a errors+=1
    )
) else (
    echo [X] flex 未安装
    set /a errors+=1
)
echo.

echo [3] 检查 bison...
where bison >nul 2>nul
if %errorlevel% equ 0 (
    bison --version 2>&1 | findstr /C:"bison" >nul
    if %errorlevel% equ 0 (
        echo [OK] bison 已安装
        for /f "tokens=*" %%i in ('bison --version 2^>^&1') do (
            echo     %%i
            goto :bison_done
        )
        :bison_done
    ) else (
        echo [X] bison 检测失败
        set /a errors+=1
    )
) else (
    echo [X] bison 未安装
    set /a errors+=1
)
echo.

echo [4] 检查 make...
where make >nul 2>nul
if %errorlevel% equ 0 (
    make --version 2>&1 | findstr /C:"GNU Make" >nul
    if %errorlevel% equ 0 (
        echo [OK] make 已安装
        for /f "tokens=*" %%i in ('make --version 2^>^&1') do (
            echo     %%i
            goto :make_done
        )
        :make_done
    ) else (
        echo [X] make 检测失败
        set /a errors+=1
    )
) else (
    echo [X] make 未安装
    set /a errors+=1
)
echo.

echo ==========================================
if %errors% equ 0 (
    echo [SUCCESS] 所有依赖都已正确安装！
    echo.
    echo 现在可以运行: make
) else (
    echo [ERROR] 检测到 %errors% 个问题
    echo.
    echo 请查看 SETUP_PATH.md 了解如何添加MSYS2路径
    echo.
    echo 快速方法: 运行 setup_env.bat
)
echo ==========================================
echo.

if %errors% equ 0 (
    set /p build="是否立即构建项目? (Y/N): "
    if /i "%build%"=="Y" (
        echo.
        echo 开始构建...
        make
    )
)

pause

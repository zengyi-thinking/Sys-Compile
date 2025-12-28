#!/bin/bash
# 安装flex和bison的脚本

echo "========================================="
echo "Sys编译器 - 依赖安装脚本"
echo "========================================="
echo ""

# 检查是否在MSYS2环境中
if [ -f "/etc/msys2-release" ]; then
    echo "检测到MSYS2环境"
    echo ""
    echo "正在安装flex和bison..."
    
    pacman -S mingw-w64-x86_64-flex mingw-w64-x86_64-bison --noconfirm
    
    if [ $? -eq 0 ]; then
        echo ""
        echo "========================================="
        echo "安装成功！"
        echo "========================================="
        echo ""
        echo "请重启MSYS2终端，然后运行："
        echo "  make"
        echo ""
    else
        echo "安装失败，请手动运行："
        echo "  pacman -S mingw-w64-x86_64-flex mingw-w64-x86_64-bison"
    fi
else
    echo "错误: 未检测到MSYS2环境"
    echo ""
    echo "请在MSYS2 MinGW 64-bit终端中运行此脚本"
    echo ""
fi

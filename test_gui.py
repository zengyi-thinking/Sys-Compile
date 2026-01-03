#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
测试编译器所有阶段的输出
"""

import subprocess
import sys

def run_compiler(input_file, mode):
    cmd = ['build/sysc.exe']
    if mode:
        cmd.append(mode)
    cmd.append(input_file)
    
    result = subprocess.run(cmd, capture_output=True, timeout=10)
    return result.stdout

def main():
    modes = [
        ('', '1. 词法分析'),
        ('-ast', '2. 抽象语法树'),
        ('-semantic', '3. 语义分析'),
        ('-ir', '4. 中间代码'),
        ('-optimize', '5. 代码优化'),
        ('-asm', '6. 目标代码'),
    ]
    
    print("Sys编译器 v3.0 - 编译阶段测试")
    print("=" * 60)
    
    for mode, name in modes:
        print(f"\n{name}")
        print("-" * 40)
        
        output = run_compiler('examples/test_basic.sy', mode)
        text = output.decode('utf-8', errors='replace')
        
        lines = [l for l in text.strip().split('\n') if l.strip()]
        print(f"输出 {len(lines)} 行:")
        for l in lines[:8]:
            print(f"  {l}")
        if len(lines) > 8:
            print(f"  ... 还有 {len(lines) - 8} 行")
    
    print("\n" + "=" * 60)
    print("测试完成!")
    print("\n运行 GUI: python gui.py")

if __name__ == "__main__":
    main()

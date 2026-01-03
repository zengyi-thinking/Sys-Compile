#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Sys编译器 GUI 界面 v3.0
现代化的编译器前端界面
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, filedialog, messagebox
import subprocess
import os
import sys
import threading
import time

class ModernButton(tk.Canvas):
    """现代化按钮"""
    def __init__(self, parent, text, color, hover_color, command=None, width=120, height=36):
        super().__init__(parent, width=width, height=height, bg=parent['bg'],
                        highlightthickness=0, relief=tk.FLAT)
        self.command = command
        self.text = text
        self.color = color
        self.hover_color = hover_color
        self.original_color = color

        self.rect = self.create_rectangle(0, 0, width, height, fill=color, outline="")
        self.text_id = self.create_text(width//2, height//2, text=text, fill="white",
                                        font=("Microsoft YaHei UI", 10, "bold"))

        self.bind("<Enter>", self.on_enter)
        self.bind("<Leave>", self.on_leave)
        self.bind("<Button-1>", self.on_click)

    def on_enter(self, event):
        self.config(cursor="hand2")
        self.itemconfig(self.rect, fill=self.hover_color)

    def on_leave(self, event):
        self.config(cursor="")
        self.itemconfig(self.rect, fill=self.original_color)

    def on_click(self, event):
        if self.command:
            self.command()


class StageCard(tk.Frame):
    """编译阶段卡片"""
    def __init__(self, parent, title, icon, color):
        super().__init__(parent, bg="#2b2b2b", highlightthickness=0)
        self.color = color
        self.status = "pending"  # pending, running, completed, error

        self.canvas = tk.Canvas(self, width=160, height=80, bg="#2b2b2b",
                               highlightthickness=0, relief=tk.FLAT)
        self.canvas.pack(fill=tk.BOTH, expand=True)

        # 背景
        self.bg_rect = self.canvas.create_rectangle(0, 0, 160, 80,
                                                  fill="#353535", outline="", tags="bg")

        # 图标圈
        self.icon_circle = self.canvas.create_oval(70, 10, 90, 30,
                                                   fill="#444", outline="", tags="icon")

        # 标题
        self.canvas.create_text(80, 55, text=title, fill="#888",
                               font=("Microsoft YaHei UI", 9), tags="title")

    def set_status(self, status):
        """设置状态"""
        self.status = status
        if status == "running":
            self.canvas.itemconfig("bg", fill="#3a3a3a")
            self.canvas.itemconfig("icon", fill="#007acc")
        elif status == "completed":
            self.canvas.itemconfig("bg", fill="#1a3a1a")
            self.canvas.itemconfig("icon", fill="#107c10")
        elif status == "error":
            self.canvas.itemconfig("bg", fill="#3a1a1a")
            self.canvas.itemconfig("icon", fill="#d13438")
        else:
            self.canvas.itemconfig("bg", fill="#353535")
            self.canvas.itemconfig("icon", fill="#444")


class SysCompilerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sys编译器 v3.0")
        self.root.geometry("1400x850")
        self.root.configure(bg="#1e1e1e")

        # 编译器路径
        self.compiler_path = "build/sysc.exe"

        # 当前文件和代码
        self.current_file = None
        self.is_compiling = False

        # 编译阶段状态
        self.stages = {}

        # 示例文件
        self.examples = {
            "📝 基础语法": "examples/test_basic.sy",
            "🧮 表达式": "examples/test_expr.sy",
            "🔀 条件语句": "examples/test_if.sy",
            "🔁 循环语句": "examples/test_while.sy",
            "⚙️ 函数测试": "examples/test_func.sy",
            "📊 数组测试": "examples/test_array.sy",
            "🎯 演示程序": "examples/demo.sy",
            "🔧 综合测试": "examples/test.sy",
            "🔄 类型转换": "examples/test_cast.sy",
            "🧱 多维数组": "examples/test_multidim.sy",
            "🔒 常量测试": "examples/test_const.sy",
            "📋 数组参数": "examples/test_array_param.sy",
        }

        self.setup_ui()
        self.load_example("🎯 演示程序")

    def setup_ui(self):
        """设置UI"""
        # 顶部标题栏
        self.create_header()

        # 主内容区
        main_container = tk.Frame(self.root, bg="#1e1e1e")
        main_container.pack(fill=tk.BOTH, expand=True, padx=20, pady=(10, 10))

        # 左侧面板（文件列表 + 代码编辑）
        left_panel = tk.Frame(main_container, bg="#1e1e1e")
        left_panel.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # 右侧面板（编译阶段卡片 + 输出）
        right_panel = tk.Frame(main_container, bg="#1e1e1e", width=450)
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, padx=(15, 0))
        right_panel.pack_propagate(False)

        # 填充左侧
        self.create_left_panel(left_panel)

        # 填充右侧
        self.create_right_panel(right_panel)

    def create_header(self):
        """创建顶部标题栏"""
        header = tk.Frame(self.root, bg="#252526", height=60)
        header.pack(fill=tk.X)
        header.pack_propagate(False)

        # Logo和标题
        logo_frame = tk.Frame(header, bg="#252526")
        logo_frame.pack(side=tk.LEFT, padx=20)

        tk.Label(logo_frame, text="⚡", bg="#252526", fg="#007acc",
                font=("Segoe UI Emoji", 24)).pack(side=tk.LEFT)

        title_frame = tk.Frame(header, bg="#252526")
        title_frame.pack(side=tk.LEFT, padx=10)

        tk.Label(title_frame, text="Sys编译器", bg="#252526", fg="#ffffff",
                font=("Microsoft YaHei UI", 16, "bold")).pack()
        tk.Label(title_frame, text="v3.0 - 现代化编译器前端", bg="#252526",
                fg="#888888", font=("Microsoft YaHei UI", 9)).pack()

        # 右侧工具按钮
        btn_frame = tk.Frame(header, bg="#252526")
        btn_frame.pack(side=tk.RIGHT, padx=20)

        # 文件操作按钮
        self.create_tool_button(btn_frame, "📂", "打开文件", "#0078d7", self.open_file)
        self.create_tool_button(btn_frame, "💾", "保存", "#3a3a3a", self.save_file)
        self.create_tool_button(btn_frame, "▶️", "编译", "#107c10", self.compile_all)
        self.create_tool_button(btn_frame, "🧹", "清空", "#d13438", self.clear_all)

    def create_tool_button(self, parent, icon, tooltip, color, command):
        """创建工具按钮"""
        btn = tk.Button(parent, text=icon, bg=color, fg="white",
                       font=("Segoe UI Emoji", 14), relief=tk.FLAT,
                       width=3, height=1, cursor="hand2", command=command,
                       highlightthickness=0)
        btn.pack(side=tk.LEFT, padx=3)
        # 简单的tooltip
        btn.tooltip = tooltip
        return btn

    def create_left_panel(self, parent):
        """创建左侧面板"""
        # 示例文件栏
        example_bar = tk.Frame(parent, bg="#252526", height=40)
        example_bar.pack(fill=tk.X)
        example_bar.pack_propagate(False)

        tk.Label(example_bar, text="📚 示例文件", bg="#252526", fg="#007acc",
                font=("Microsoft YaHei UI", 10, "bold")).pack(side=tk.LEFT, padx=15, pady=10)

        # 示例文件下拉框
        self.example_var = tk.StringVar(value="选择示例文件...")
        example_combo = ttk.Combobox(example_bar, textvariable=self.example_var,
                                     values=list(self.examples.keys()),
                                     state="readonly", width=25,
                                     font=("Microsoft YaHei UI", 9))
        example_combo.pack(side=tk.RIGHT, padx=15, pady=8)
        example_combo.bind("<<ComboboxSelected>>", lambda e: self.on_example_selected())

        # 代码编辑器
        editor_frame = tk.Frame(parent, bg="#1e1e1e")
        editor_frame.pack(fill=tk.BOTH, expand=True, pady=(10, 0))

        # 编辑器标题
        editor_header = tk.Frame(editor_frame, bg="#252526", height=35)
        editor_header.pack(fill=tk.X)
        editor_header.pack_propagate(False)

        self.file_label = tk.Label(editor_header, text="untitled.sy",
                                   bg="#252526", fg="#cccccc", anchor="w",
                                   font=("Consolas", 10))
        self.file_label.pack(side=tk.LEFT, padx=15, pady=8)

        # 代码编辑区
        self.code_editor = scrolledtext.ScrolledText(
            editor_frame,
            wrap=tk.NONE,
            font=("Consolas", 12),
            bg="#1e1e1e",
            fg="#d4d4d4",
            insertbackground="#ffffff",
            selectbackground="#264f78",
            selectforeground="#ffffff",
            borderwidth=0,
            highlightthickness=1,
            highlightbackground="#333",
            highlightcolor="#007acc",
            padx=10,
            pady=10
        )
        self.code_editor.pack(fill=tk.BOTH, expand=True)

        # 行号
        self.line_numbers = tk.Text(editor_frame, width=4, padx=5, pady=10,
                                     font=("Consolas", 12), state='disabled',
                                     bg="#1e1e1e", fg="#555", borderwidth=0,
                                     highlightthickness=0)
        self.line_numbers.pack(side=tk.LEFT, fill=tk.Y)
        self.code_editor.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

    def create_right_panel(self, parent):
        """创建右侧面板"""
        # 编译流程标题
        flow_header = tk.Frame(parent, bg="#1e1e1e", height=40)
        flow_header.pack(fill=tk.X)
        flow_header.pack_propagate(False)

        tk.Label(flow_header, text="🔄 编译流程", bg="#1e1e1e", fg="#007acc",
                font=("Microsoft YaHei UI", 11, "bold")).pack(side=tk.LEFT, pady=10)

        # 编译阶段卡片网格
        cards_frame = tk.Frame(parent, bg="#1e1e1e")
        cards_frame.pack(fill=tk.X, pady=(5, 15))

        # 第一行
        row1 = tk.Frame(cards_frame, bg="#1e1e1e")
        row1.pack(fill=tk.X, pady=2)

        self.stage_lexical = StageCard(row1, "词法分析", "📝", "#4ec9b0")
        self.stage_lexical.pack(side=tk.LEFT, padx=2)

        self.stage_syntax = StageCard(row1, "语法分析", "🌳", "#4ec9b0")
        self.stage_syntax.pack(side=tk.LEFT, padx=2)

        self.stage_semantic = StageCard(row1, "语义分析", "✓", "#4ec9b0")
        self.stage_semantic.pack(side=tk.LEFT, padx=2)

        # 第二行
        row2 = tk.Frame(cards_frame, bg="#1e1e1e")
        row2.pack(fill=tk.X, pady=2)

        self.stage_ir = StageCard(row2, "中间代码", "📋", "#4ec9b0")
        self.stage_ir.pack(side=tk.LEFT, padx=2)

        self.stage_optimize = StageCard(row2, "代码优化", "⚡", "#4ec9b0")
        self.stage_optimize.pack(side=tk.LEFT, padx=2)

        self.stage_target = StageCard(row2, "目标代码", "🎯", "#4ec9b0")
        self.stage_target.pack(side=tk.LEFT, padx=2)

        # 输出区域
        output_frame = tk.Frame(parent, bg="#1e1e1e")
        output_frame.pack(fill=tk.BOTH, expand=True)

        # 输出标签页
        self.notebook = ttk.Notebook(output_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        # 配置Notebook样式
        style = ttk.Style()
        style.theme_use('clam')
        style.configure("TNotebook", background="#1e1e1e", borderwidth=0)
        style.configure("TNotebook.Tab", background="#2d2d2d", foreground="#cccccc",
                       padding=[15, 8], borderwidth=0)
        style.map("TNotebook.Tab",
                  background=[("selected", "#1e1e1e")],
                  foreground=[("selected", "#007acc")])

        # 创建标签页
        self.create_output_tabs()

    def create_output_tabs(self):
        """创建输出标签页"""
        # 编译输出
        compile_frame = tk.Frame(self.notebook, bg="#1e1e1e")
        self.notebook.add(compile_frame, text="  编译输出  ")

        self.compile_output = scrolledtext.ScrolledText(
            compile_frame, wrap=tk.WORD, font=("Consolas", 10),
            bg="#0c0c0c", fg="#4ec9b0", borderwidth=0,
            highlightthickness=0, padx=10, pady=10
        )
        self.compile_output.pack(fill=tk.BOTH, expand=True)

        # Token输出
        token_frame = tk.Frame(self.notebook, bg="#1e1e1e")
        self.notebook.add(token_frame, text="  Token  ")

        self.token_output = scrolledtext.ScrolledText(
            token_frame, wrap=tk.NONE, font=("Consolas", 10),
            bg="#0c0c0c", fg="#d4d4d4", borderwidth=0,
            highlightthickness=0, padx=10, pady=10
        )
        self.token_output.pack(fill=tk.BOTH, expand=True)

        # 配置Token颜色标签
        self.token_output.tag_config("type", foreground="#4ec9b0")
        self.token_output.tag_config("value", foreground="#ffffff")
        self.token_output.tag_config("line", foreground="#858585")
        self.token_output.tag_config("header", foreground="#007acc", font=("Consolas", 10, "bold"))

        # AST输出
        ast_frame = tk.Frame(self.notebook, bg="#1e1e1e")
        self.notebook.add(ast_frame, text="  AST  ")

        self.ast_output = scrolledtext.ScrolledText(
            ast_frame, wrap=tk.WORD, font=("Consolas", 9),
            bg="#0c0c0c", fg="#d4d4d4", borderwidth=0,
            highlightthickness=0, padx=10, pady=10
        )
        self.ast_output.pack(fill=tk.BOTH, expand=True)

        # 汇编输出
        asm_frame = tk.Frame(self.notebook, bg="#1e1e1e")
        self.notebook.add(asm_frame, text="  汇编代码  ")

        self.asm_output = scrolledtext.ScrolledText(
            asm_frame, wrap=tk.WORD, font=("Consolas", 10),
            bg="#0c0c0c", fg="#d4d4d4", borderwidth=0,
            highlightthickness=0, padx=10, pady=10
        )
        self.asm_output.pack(fill=tk.BOTH, expand=True)

    def on_example_selected(self):
        """示例文件选择事件"""
        selected = self.example_var.get()
        if selected in self.examples:
            self.load_example(selected)

    def load_example(self, name):
        """加载示例文件"""
        if name in self.examples:
            filepath = self.examples[name]
            if os.path.exists(filepath):
                try:
                    with open(filepath, 'r', encoding='utf-8') as f:
                        content = f.read()
                    self.code_editor.delete(1.0, tk.END)
                    self.code_editor.insert(1.0, content)
                    self.current_file = filepath
                    self.file_label.config(text=os.path.basename(filepath))
                    self.update_line_numbers()
                except Exception as e:
                    messagebox.showerror("错误", f"无法加载文件: {e}")

    def update_line_numbers(self):
        """更新行号"""
        content = self.code_editor.get(1.0, tk.END)
        lines = content.count('\n')
        line_nums = '\n'.join(str(i) for i in range(1, lines + 2))
        self.line_numbers.config(state='normal')
        self.line_numbers.delete(1.0, tk.END)
        self.line_numbers.insert(1.0, line_nums)
        self.line_numbers.config(state='disabled')

    def open_file(self):
        """打开文件"""
        filename = filedialog.askopenfilename(
            title="选择Sys源文件",
            filetypes=[("Sys文件", "*.sy"), ("所有文件", "*.*")],
            initialdir="examples"
        )
        if filename:
            try:
                with open(filename, 'r', encoding='utf-8') as f:
                    content = f.read()
                self.code_editor.delete(1.0, tk.END)
                self.code_editor.insert(1.0, content)
                self.current_file = filename
                self.file_label.config(text=os.path.basename(filename))
                self.update_line_numbers()
            except Exception as e:
                messagebox.showerror("错误", f"无法打开文件: {e}")

    def save_file(self):
        """保存文件"""
        if not self.current_file:
            filename = filedialog.asksaveasfilename(
                title="保存Sys源文件",
                filetypes=[("Sys文件", "*.sy"), ("所有文件", "*.*")],
                defaultextension=".sy"
            )
        else:
            filename = self.current_file

        if filename:
            try:
                with open(filename, 'w', encoding='utf-8') as f:
                    f.write(self.code_editor.get(1.0, tk.END))
                self.current_file = filename
                self.file_label.config(text=os.path.basename(filename))
                messagebox.showinfo("成功", "文件保存成功")
            except Exception as e:
                messagebox.showerror("错误", f"无法保存文件: {e}")

    def clear_all(self):
        """清空所有内容"""
        self.code_editor.delete(1.0, tk.END)
        self.compile_output.delete(1.0, tk.END)
        self.token_output.delete(1.0, tk.END)
        self.ast_output.delete(1.0, tk.END)
        self.asm_output.delete(1.0, tk.END)
        self.file_label.config(text="untitled.sy")

        # 重置所有阶段状态
        for stage in [self.stage_lexical, self.stage_syntax, self.stage_semantic,
                      self.stage_ir, self.stage_optimize, self.stage_target]:
            stage.set_status("pending")

    def append_output(self, text, color="#4ec9b0"):
        """追加编译输出"""
        self.compile_output.insert(tk.END, text, color)
        self.compile_output.see(tk.END)
        self.compile_output.tag_config(color, foreground=color)

    def run_compiler_stage(self, stage_name, args):
        """运行编译器单个阶段"""
        try:
            result = subprocess.run(
                [self.compiler_path] + args,
                capture_output=True,
                text=True,
                timeout=10,
                encoding='utf-8',
                errors='ignore'
            )
            return result.stdout + result.stderr, result.returncode == 0
        except subprocess.TimeoutExpired:
            return f"\n错误: {stage_name} 超时\n", False
        except Exception as e:
            return f"\n错误: {stage_name} - {str(e)}\n", False

    def compile_all(self):
        """执行完整编译"""
        code = self.code_editor.get(1.0, tk.END).strip()
        if not code:
            messagebox.showwarning("警告", "请先输入或选择代码")
            return

        if self.is_compiling:
            return

        self.is_compiling = True

        # 清空输出
        self.compile_output.delete(1.0, tk.END)
        self.token_output.delete(1.0, tk.END)
        self.ast_output.delete(1.0, tk.END)
        self.asm_output.delete(1.0, tk.END)

        # 保存临时文件
        temp_file = "temp_gui_compile.sy"
        with open(temp_file, 'w', encoding='utf-8') as f:
            f.write(code)

        def run_compilation():
            stages = [
                ("词法分析", ["-lex", temp_file], self.stage_lexical),
                ("语法分析", ["-ast", temp_file], self.stage_syntax),
                ("语义分析", ["-semantic", temp_file], self.stage_semantic),
                ("中间代码", ["-ir", temp_file], self.stage_ir),
                ("代码优化", ["-optimize", temp_file], self.stage_optimize),
                ("目标代码", ["-asm", temp_file], self.stage_target),
            ]

            all_success = True

            for stage_name, args, stage_widget in stages:
                self.root.after(0, lambda s=stage_widget: s.set_status("running"))
                self.root.after(0, lambda n=stage_name: self.append_output(f"\n▶ {n}...\n", "#007acc"))

                output, success = self.run_compiler_stage(stage_name, args)

                if success:
                    self.root.after(0, lambda s=stage_widget: s.set_status("completed"))
                    self.root.after(0, lambda n=stage_name: self.append_output(f"✓ {n} 完成\n", "#107c10"))
                else:
                    self.root.after(0, lambda s=stage_widget: s.set_status("error"))
                    self.root.after(0, lambda n=stage_name: self.append_output(f"✗ {n} 失败\n", "#d13438"))
                    all_success = False

                # 将输出分配到对应标签页
                self.root.after(0, self.distribute_output, stage_name, output)
                time.sleep(0.3)  # 添加小延迟让动画更流畅

            self.root.after(0, lambda: self.append_output("\n" + "="*50 + "\n", "#888"))
            if all_success:
                self.root.after(0, lambda: self.append_output("✓ 编译完成!\n", "#107c10"))
            else:
                self.root.after(0, lambda: self.append_output("⚠ 编译完成，但有错误\n", "#d83b01"))

            self.root.after(0, lambda: setattr(self, 'is_compiling', False))

            # 清理临时文件
            for ext in ['.sy', '.s', '.o']:
                f = temp_file.replace('.sy', ext)
                if os.path.exists(f):
                    os.remove(f)

        # 在新线程中运行编译
        thread = threading.Thread(target=run_compilation, daemon=True)
        thread.start()

    def distribute_output(self, stage_name, output):
        """将输出分配到对应的标签页"""
        if not output:
            return

        if stage_name == "词法分析":
            # 解析并格式化Token
            self.display_tokens(output)
        elif stage_name == "语法分析":
            self.ast_output.delete(1.0, tk.END)
            self.ast_output.insert(1.0, output)
        elif stage_name == "语义分析":
            self.compile_output.insert(tk.END, "\n" + output)
        elif stage_name == "目标代码":
            # 提取汇编代码
            self.extract_assembly(output)
        else:
            self.compile_output.insert(tk.END, "\n" + output)

    def display_tokens(self, output):
        """显示Token"""
        self.token_output.delete(1.0, tk.END)
        self.token_output.insert(tk.END, f"{'类型':<12} {'值':<20} {'行号':<6}\n", "header")
        self.token_output.insert(tk.END, "-" * 45 + "\n", "header")

        lines = output.split('\n')
        token_count = 0

        for line in lines:
            line = line.strip()
            if not line or line.startswith('=') or line.startswith('错误'):
                continue

            # 尝试解析token行
            parts = line.split()
            if len(parts) >= 2:
                if parts[0] in ['KEYWORD', 'OPERATOR', 'SYMBOL', 'NUMBER', 'FLOAT',
                                'IDENTIFIER', 'UNKNOWN']:
                    token_type = parts[0]
                    token_value = parts[1] if len(parts) > 1 else ""
                    line_num = parts[2] if len(parts) > 2 else "-"

                    self.token_output.insert(tk.END, f"{token_type:<12} ", "type")
                    self.token_output.insert(tk.END, f"{token_value:<20} ", "value")
                    self.token_output.insert(tk.END, f"{line_num:<6}\n", "line")
                    token_count += 1

        self.token_output.insert(tk.END, "\n", "header")
        self.token_output.insert(tk.END, f"总计: {token_count} 个Token\n", "header")

    def extract_assembly(self, output):
        """提取汇编代码"""
        self.asm_output.delete(1.0, tk.END)
        lines = output.split('\n')
        in_asm = False

        for line in lines:
            if '汇编' in line or 'Assembly' in line or '.section' in line:
                in_asm = True
            if in_asm:
                self.asm_output.insert(tk.END, line + '\n')
                if line.strip().startswith('编译完成'):
                    break


def main():
    root = tk.Tk()
    app = SysCompilerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()

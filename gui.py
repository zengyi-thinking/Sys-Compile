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
    """编译阶段卡片 - 自适应版本"""
    def __init__(self, parent, title, icon, color):
        super().__init__(parent, bg="#2b2b2b", highlightthickness=0,
                       padx=5, pady=5)
        self.color = color
        self.status = "pending"  # pending, running, completed, error
        self.title_text = title

        # 主容器
        self.main_frame = tk.Frame(self, bg="#353535", relief=tk.FLAT)
        self.main_frame.pack(fill=tk.BOTH, expand=True)

        # 图标标签
        self.icon_label = tk.Label(self.main_frame, text=icon, bg="#353535",
                                  fg="#444", font=("Segoe UI Emoji", 16))
        self.icon_label.pack(pady=(8, 2))

        # 标题标签
        self.title_label = tk.Label(self.main_frame, text=title, bg="#353535",
                                   fg="#888", font=("Microsoft YaHei UI", 8))
        self.title_label.pack(pady=(0, 5))

        # 绑定大小变化事件
        self.bind("<Configure>", self._on_resize)

    def _on_resize(self, event):
        """窗口大小变化时重绘"""
        pass

    def set_status(self, status):
        """设置状态"""
        self.status = status
        if status == "running":
            self.main_frame.config(bg="#3a3a3a")
            self.icon_label.config(bg="#3a3a3a", fg="#007acc")
        elif status == "completed":
            self.main_frame.config(bg="#1a3a1a")
            self.icon_label.config(bg="#1a3a1a", fg="#107c10")
        elif status == "error":
            self.main_frame.config(bg="#3a1a1a")
            self.icon_label.config(bg="#3a1a1a", fg="#d13438")
        else:
            self.main_frame.config(bg="#353535")
            self.icon_label.config(bg="#353535", fg="#444")


class SysCompilerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Sys编译器 v3.0")
        self.root.geometry("1400x850")
        self.root.configure(bg="#1e1e1e")

        # 确保工作目录正确（切换到脚本所在目录）
        import os
        script_dir = os.path.dirname(os.path.abspath(__file__))
        os.chdir(script_dir)

        # 绑定窗口大小变化事件
        self.root.bind('<Configure>', self.on_window_resize)

        # 编译器路径 - 自动检测平台
        import platform
        import sys
        if platform.system() == 'Windows':
            # Windows使用相对路径，不带./
            self.compiler_path = "build/sysc.exe"
        else:
            self.compiler_path = "build/sysc"

        # 检查编译器是否存在
        self.compiler_available = os.path.exists(self.compiler_path)

        # 当前文件和代码
        self.current_file = None
        self.is_compiling = False
        self.last_ir_output = ""

        # 编译阶段状态
        self.stages = {}

        # 示例文件
        self.examples = {
            "📝 基础语法": "examples/test_basic.sy",
            "🔄 类型转换": "examples/test_cast.sy",
            "🧱 多维数组": "examples/test_multidim.sy",
            "🔒 常量测试": "examples/test_const.sy",
            "📋 数组参数": "examples/test_array_param.sy",
            "🔁 循环语句": "examples/test_while.sy",
            "⚙️ 函数测试": "examples/test_func.sy",
            "🎯 演示程序": "examples/demo.sy",
            "🔧 综合测试": "examples/test.sy",
        }

        self.setup_ui()
        self.load_example("🎯 演示程序")

        # 显示编译器状态
        if not self.compiler_available:
            self.append_output("⚠ 警告: 未找到编译器 " + self.compiler_path + "\n", "#d83b01")
            self.append_output("请先运行 'make' 编译编译器\n", "#d83b01")

    def setup_ui(self):
        """设置UI"""
        # 顶部标题栏
        self.create_header()

        # 主内容区
        main_container = tk.Frame(self.root, bg="#1e1e1e")
        main_container.pack(fill=tk.BOTH, expand=True, padx=20, pady=(10, 10))

        # 创建分割器容器
        paned_window = tk.PanedWindow(main_container, bg="#1e1e1e",
                                     orient=tk.HORIZONTAL, sashwidth=4,
                                     sashrelief=tk.RAISED)
        paned_window.pack(fill=tk.BOTH, expand=True)

        # 左侧面板（文件列表 + 代码编辑）
        left_panel = tk.Frame(paned_window, bg="#1e1e1e")
        paned_window.add(left_panel, minsize=400)

        # 右侧面板（编译阶段卡片 + 输出）
        right_panel = tk.Frame(paned_window, bg="#1e1e1e")
        paned_window.add(right_panel, minsize=400)

        # 保存paned_window引用以便调整大小
        self.paned_window = paned_window

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

        # 编译阶段卡片容器 - 使用grid布局自适应
        cards_frame = tk.Frame(parent, bg="#1e1e1e")
        cards_frame.pack(fill=tk.X, pady=(5, 15))

        # 配置grid权重，让列等宽
        cards_frame.grid_columnconfigure(0, weight=1)
        cards_frame.grid_columnconfigure(1, weight=1)
        cards_frame.grid_columnconfigure(2, weight=1)
        cards_frame.grid_rowconfigure(0, weight=1)
        cards_frame.grid_rowconfigure(1, weight=1)

        # 第一行 - 使用grid布局
        self.stage_lexical = StageCard(cards_frame, "词法分析", "📝", "#4ec9b0")
        self.stage_lexical.grid(row=0, column=0, sticky="nsew", padx=2, pady=2)

        self.stage_syntax = StageCard(cards_frame, "语法分析", "🌳", "#4ec9b0")
        self.stage_syntax.grid(row=0, column=1, sticky="nsew", padx=2, pady=2)

        self.stage_semantic = StageCard(cards_frame, "语义分析", "✓", "#4ec9b0")
        self.stage_semantic.grid(row=0, column=2, sticky="nsew", padx=2, pady=2)

        # 第二行
        self.stage_ir = StageCard(cards_frame, "中间代码", "📋", "#4ec9b0")
        self.stage_ir.grid(row=1, column=0, sticky="nsew", padx=2, pady=2)

        self.stage_optimize = StageCard(cards_frame, "代码优化", "⚡", "#4ec9b0")
        self.stage_optimize.grid(row=1, column=1, sticky="nsew", padx=2, pady=2)

        self.stage_target = StageCard(cards_frame, "目标代码", "🎯", "#4ec9b0")
        self.stage_target.grid(row=1, column=2, sticky="nsew", padx=2, pady=2)

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
        self.last_ir_output = ""
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

    def on_window_resize(self, event):
        """窗口大小变化时的处理"""
        # 只处理主窗口的大小变化
        if event.widget == self.root:
            # 确保编辑器和输出区域能够自适应
            pass

    def run_compiler_stage(self, stage_name, args):
        """运行编译器单个阶段"""
        # 检查编译器是否可用
        if not self.compiler_available:
            return f"\n错误: 编译器不可用，请先运行 'make' 编译编译器\n", False

        try:
            result = subprocess.run(
                [self.compiler_path] + args,
                capture_output=True,
                text=True,
                timeout=30,
                encoding='utf-8',
                errors='ignore'
            )
            output = result.stdout + result.stderr
            success = result.returncode == 0

            # 如果输出为空，返回提示信息
            if not output.strip():
                output = f"(无输出)\n"

            return output, success
        except subprocess.TimeoutExpired:
            return f"\n错误: {stage_name} 超时（超过30秒）\n", False
        except FileNotFoundError:
            self.compiler_available = False
            return f"\n错误: 找不到编译器 '{self.compiler_path}'\n请先运行 'make' 编译编译器\n", False
        except Exception as e:
            return f"\n错误: {stage_name} - {str(e)}\n", False

    def compile_all(self):
        """执行完整编译"""
        # 检查编译器是否可用
        if not self.compiler_available:
            messagebox.showerror("错误", "编译器不可用！\n\n请先在项目目录下运行 'make' 命令编译编译器。")
            return

        code = self.code_editor.get(1.0, tk.END).strip()
        if not code:
            messagebox.showwarning("警告", "请先输入或选择代码")
            return

        if self.is_compiling:
            messagebox.showinfo("提示", "编译正在进行中，请稍候...")
            return

        self.is_compiling = True

        # 清空输出
        self.compile_output.delete(1.0, tk.END)
        self.token_output.delete(1.0, tk.END)
        self.ast_output.delete(1.0, tk.END)
        self.asm_output.delete(1.0, tk.END)

        # 重置所有阶段状态
        for stage in [self.stage_lexical, self.stage_syntax, self.stage_semantic,
                      self.stage_ir, self.stage_optimize, self.stage_target]:
            stage.set_status("pending")

        # 保存临时文件
        temp_file = "temp_gui_compile.sy"
        try:
            with open(temp_file, 'w', encoding='utf-8') as f:
                f.write(code)
        except Exception as e:
            messagebox.showerror("错误", f"无法保存临时文件: {e}")
            self.is_compiling = False
            return

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
            error_occurred = False

            for idx, (stage_name, args, stage_widget) in enumerate(stages):
                # 设置状态为运行中
                self.root.after(0, lambda s=stage_widget: s.set_status("running"))
                self.root.after(0, lambda n=stage_name: self.append_output(f"\n▶ {n}...\n", "#007acc"))

                # 运行编译器阶段
                output, success = self.run_compiler_stage(stage_name, args)

                if success:
                    self.root.after(0, lambda s=stage_widget: s.set_status("completed"))
                    self.root.after(0, lambda n=stage_name: self.append_output(f"✓ {n} 完成\n", "#107c10"))
                else:
                    self.root.after(0, lambda s=stage_widget: s.set_status("error"))
                    self.root.after(0, lambda n=stage_name: self.append_output(f"✗ {n} 失败\n", "#d13438"))
                    all_success = False
                    error_occurred = True

                # 将输出分配到对应标签页
                self.root.after(0, self.distribute_output, stage_name, output)

                # 如果出错，询问是否继续
                if not success and idx < len(stages) - 1:
                    time.sleep(0.5)
                    # 继续执行其他阶段

                time.sleep(0.3)  # 添加小延迟让动画更流畅

            # 最终状态
            self.root.after(0, lambda: self.append_output("\n" + "="*50 + "\n", "#888"))
            if all_success:
                self.root.after(0, lambda: self.append_output("✓ 编译完成!\n", "#107c10"))
            elif error_occurred:
                self.root.after(0, lambda: self.append_output("⚠ 编译完成，但有错误\n", "#d83b01"))

            self.root.after(0, lambda: setattr(self, 'is_compiling', False))

            # 清理临时文件
            try:
                if os.path.exists(temp_file):
                    os.remove(temp_file)
                # 清理可能生成的汇编文件
                asm_file = temp_file.replace('.sy', '.s')
                if os.path.exists(asm_file):
                    os.remove(asm_file)
            except:
                pass

        # 在新线程中运行编译
        thread = threading.Thread(target=run_compilation, daemon=True)
        thread.start()

    def distribute_output(self, stage_name, output):
        """将输出分配到对应的标签页"""
        if not output or output.strip() == "(无输出)":
            return

        # 转换格式以匹配设计报告模板
        if stage_name == "词法分析":
            self.format_lexical_output(output)
        elif stage_name == "语法分析":
            self.format_syntax_output(output)
        elif stage_name == "语义分析":
            self.format_semantic_output(output)
        elif stage_name == "中间代码":
            self.last_ir_output = output
            self.format_ir_output(output)
        elif stage_name == "代码优化":
            self.format_optimize_output(output)
        elif stage_name == "目标代码":
            self.format_asm_output(output)

        # 同时也填充到专门的标签页
        if stage_name == "词法分析":
            self.display_tokens(output)
        elif stage_name == "语法分析":
            self.ast_output.delete(1.0, tk.END)
            self.extract_section(output, "抽象语法树", self.ast_output)
        elif stage_name == "目标代码":
            self.extract_assembly(output)

    def format_lexical_output(self, output):
        """格式化词法分析输出，匹配设计报告格式"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "词法分析器将源代码分解成Token序列：\n\n", "#4ec9b0")

        # 解析Token表格并转换为设计报告格式
        lines = output.split('\n')
        token_sequence = []

        for line in lines:
            if line.startswith('|') and not line.startswith('|+'):
                parts = [p.strip() for p in line.split('|')]
                if len(parts) >= 3 and parts[1] not in ['Token 类型', '---']:
                    token_type = parts[1]
                    token_value = parts[2]
                    line_num = parts[3] if len(parts) > 3 else "1"

                    # 转换Token类型名称
                    type_mapping = {
                        'KEYWORD': f'KEYWORD_{token_value.upper()}',
                        'IDENTIFIER': f'IDENTIFIER({token_value})',
                        'NUMBER': f'NUMBER({token_value})',
                        'FLOAT': f'FLOAT({token_value})',
                        'OPERATOR': f'OPERATOR_{token_value}',
                        'SYMBOL': f'SYMBOL_{token_value}',
                    }

                    if token_type in type_mapping:
                        formatted_token = type_mapping[token_type]
                    else:
                        formatted_token = f'{token_type}({token_value})'

                    token_sequence.append(formatted_token)

        # 每行显示3个Token
        for i in range(0, len(token_sequence), 3):
            line_tokens = token_sequence[i:i+3]
            formatted_line = '  '.join(line_tokens)
            self.compile_output.insert(tk.END, formatted_line + "\n", "#cccccc")

        self.compile_output.insert(tk.END, "\n", "#888")

    def format_syntax_output(self, output):
        """格式化语法分析输出"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "语法分析过程（AST）：\n\n", "#4ec9b0")

        # 修复提取AST部分的逻辑
        lines = output.split('\n')
        in_ast = False
        found_ast = False

        for line in lines:
            # 查找AST开始标记
            if '抽象语法树' in line or 'AST' in line:
                in_ast = True
                found_ast = True
                continue

            # 如果找到了AST内容
            if in_ast:
                # 跳过空行和分隔符
                if line.strip() == '' or line.strip() == '===':
                    continue

                # 遇到下一个阶段标题时停止
                if line.startswith('==============================================') and '3.' in line:
                    break

                # 提取AST行
                if line.strip():
                    self.compile_output.insert(tk.END, line + "\n", "#007acc")

        # 如果没有找到AST，显示提示
        if not found_ast:
            self.compile_output.insert(tk.END, "未找到AST内容\n", "#ff6b6b")

        self.compile_output.insert(tk.END, "\n", "#888")

    def format_semantic_output(self, output):
        """格式化语义分析输出，匹配设计报告格式"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "语义分析结果：\n\n", "#4ec9b0")

        self.compile_output.insert(tk.END, "符号表内容：\n", "#4ec9b0")

        # 提取符号表 - 修复解析逻辑
        lines = output.split('\n')
        in_table = False
        table_lines = []
        collecting_table = False

        for line in lines:
            # 查找符号表开始
            if '符号表' in line and ('(' in line or 'Symbol Table' in line):
                collecting_table = True
                continue

            # 如果正在收集符号表
            if collecting_table:
                # 符号表行以+、|、─开头
                if line.startswith('+') or line.startswith('|') or line.startswith('─'):
                    if not in_table:
                        in_table = True
                    table_lines.append(line)
                elif in_table:
                    # 符号表结束
                    in_table = False
                    collecting_table = False
                    continue

        # 如果没有找到表格，显示提示
        if not table_lines:
            self.compile_output.insert(tk.END, "未找到符号表内容\n", "#ff6b6b")
        else:
            # 显示符号表
            for line in table_lines:
                if line.strip():
                    self.compile_output.insert(tk.END, line + "\n", "#888")

        # 显示语义检查结果
        for line in lines:
            if '[OK]' in line or '语义检查通过' in line:
                self.compile_output.insert(tk.END, line + "\n", "#107c10")

        self.compile_output.insert(tk.END, "\n", "#888")

    def format_ir_output(self, output):
        """格式化中间代码输出"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "中间代码（TAC）：\n\n", "#4ec9b0")

        # 提取TAC代码 - 修复解析逻辑
        lines = output.split('\n')
        in_tac = False
        found_tac = False

        for line in lines:
            # 查找中间代码开始的标记
            if '中间代码' in line and ('TAC' in line or '三地址码' in line):
                in_tac = True
                found_tac = True
                continue

            # 如果找到了TAC内容
            if in_tac:
                # 跳过空行和分隔符
                if line.strip() == '' or line.strip() == '===':
                    continue

                # 遇到下一个阶段标题时停止
                if line.startswith('==============================================') and '5.' in line:
                    break

                # 提取实际的TAC指令
                if line.strip():
                    self.compile_output.insert(tk.END, line + "\n", "#cccccc")

        # 如果没有找到TAC内容，显示提示
        if not found_tac:
            self.compile_output.insert(tk.END, "未找到中间代码内容\n", "#ff6b6b")

        # 尝试从TAC直接计算程序返回值
        result = self.calculate_program_result(output)
        if result is not None:
            self.compile_output.insert(tk.END, "\n程序运行结果：\n", "#107c10")
            self.compile_output.insert(tk.END, f"程序执行结果: {result}\n", "#107c10")
        elif 'return' in output.lower():
            self.compile_output.insert(tk.END, "\n程序运行结果：\n", "#107c10")
            self.compile_output.insert(tk.END, "程序成功编译并可执行\n", "#cccccc")

        self.compile_output.insert(tk.END, "\n", "#888")

    def format_optimize_output(self, output):
        """格式化代码优化输出"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "代码优化：\n\n", "#4ec9b0")

        # 修复解析逻辑
        lines = output.split('\n')
        in_optimize = False
        found_optimize = False

        for line in lines:
            # 查找优化阶段开始的标记
            if '代码优化' in line and ('Optimization' in line or '优化' in line):
                in_optimize = True
                found_optimize = True
                continue

            # 如果找到了优化内容
            if in_optimize:
                # 跳过空行和分隔符
                if line.strip() == '' or line.strip() == '===':
                    continue

                # 遇到下一个阶段标题时停止
                if line.startswith('==============================================') and '6.' in line:
                    break

                # 提取优化统计信息
                if line.strip():
                    self.compile_output.insert(tk.END, line + "\n", "#cccccc")

        # 如果没有找到优化内容，显示提示
        if not found_optimize:
            self.compile_output.insert(tk.END, "未找到优化内容\n", "#ff6b6b")

        self.compile_output.insert(tk.END, "\n", "#888")

    def format_asm_output(self, output):
        """格式化汇编代码输出"""
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "最终编译输出（x86-64汇编）：\n\n", "#4ec9b0")

        # 修复解析逻辑
        lines = output.split('\n')
        in_asm = False
        found_asm = False

        for line in lines:
            # 查找汇编代码开始的标记
            if ('目标代码' in line or '汇编' in line) and ('x86' in line or 'Intel' in line or '语法' in line):
                in_asm = True
                found_asm = True
                continue

            # 如果找到了汇编代码
            if in_asm:
                # 跳过空行和分隔符
                if line.strip() == '' or line.strip() == '===':
                    continue

                # 遇到编译完成时停止
                if '编译完成' in line:
                    break

                # 提取汇编指令
                if line.strip():
                    self.compile_output.insert(tk.END, line + "\n", "#cccccc")

        # 如果没有找到汇编代码，显示提示
        if not found_asm:
            self.compile_output.insert(tk.END, "未找到汇编代码内容\n", "#ff6b6b")

        # 显示最终结果 - 修复：直接显示计算结果
        self.compile_output.insert(tk.END, "\n" + "="*60 + "\n", "#888")
        self.compile_output.insert(tk.END, "程序运行结果：\n\n", "#107c10")

        # 总是显示计算过程，即使return是0
        result_source = self.last_ir_output if self.last_ir_output else output
        calculation_process = self.get_calculation_process(result_source)
        if calculation_process:
            self.compile_output.insert(tk.END, f"计算过程: ", "#4ec9b0")
            self.compile_output.insert(tk.END, calculation_process + "\n", "#cccccc")

        # 尝试获取程序返回值
        result = self.calculate_program_result(result_source)
        if result is not None:
            # 检查是否是return 0的情况
            if result == 0 and 'return 0' in result_source:
                self.compile_output.insert(tk.END, f"程序返回值: {result}\n", "#107c10")
                self.compile_output.insert(tk.END, f"（注：return语句明确返回0，但程序计算了变量值）\n", "#888")
            else:
                self.compile_output.insert(tk.END, f"程序执行结果: {result}\n", "#107c10")
        else:
            # 如果无法计算，显示通用信息
            self.compile_output.insert(tk.END, "程序已成功编译为目标代码（x86-64汇编）\n", "#cccccc")
            self.compile_output.insert(tk.END, "程序可正常执行\n", "#cccccc")

        self.compile_output.insert(tk.END, "\n编译完成!\n", "#107c10")
        self.compile_output.insert(tk.END, "="*60 + "\n\n", "#888")

    def calculate_program_result(self, output):
        """尝试计算程序的返回值"""
        try:
            # 首先解析所有变量值
            lines = output.split('\n')
            variable_values = {}

            for line in lines:
                # 查找变量赋值
                if '=' in line and 'return' not in line and 'function' not in line:
                    parts = line.split('=')
                    if len(parts) == 2:
                        var = parts[0].strip()
                        val_expr = parts[1].strip()

                        # 计算变量的值
                        computed_val = self.evaluate_simple_expression(val_expr, variable_values)
                        if computed_val is not None:
                            variable_values[var] = computed_val

            # 然后查找return语句
            return_value = None

            for line in lines:
                # 查找中间代码中的return语句
                if 'return' in line and not line.startswith('function'):
                    # 提取return后的值
                    parts = line.split('return')
                    if len(parts) > 1:
                        return_expr = parts[1].strip()
                        # 如果是纯数字，直接返回
                        if return_expr.isdigit():
                            return_value = int(return_expr)
                        # 如果是变量名，从变量表中查找
                        elif return_expr in variable_values:
                            return_value = variable_values[return_expr]
                        # 尝试简单计算
                        else:
                            computed = self.evaluate_simple_expression(return_expr, variable_values)
                            if computed is not None:
                                return_value = computed
                    break

            return return_value
        except:
            return None

    def get_calculation_process(self, output):
        """获取计算过程说明"""
        try:
            # 从中间代码中提取所有计算步骤
            lines = output.split('\n')
            process_steps = []
            variable_values = {}

            for line in lines:
                # 查找变量赋值
                if '=' in line and 'return' not in line and 'function' not in line:
                    parts = line.split('=')
                    if len(parts) == 2:
                        var = parts[0].strip()
                        val_expr = parts[1].strip()

                        # 尝试计算表达式的值
                        computed_val = self.evaluate_simple_expression(val_expr, variable_values)
                        if computed_val is not None:
                            variable_values[var] = computed_val
                            # 添加到步骤列表
                            if 't' not in var:  # 跳过临时变量
                                process_steps.append(f"{var} = {computed_val}")

            # 返回计算过程
            if process_steps:
                return "; ".join(process_steps)
            return None
        except:
            return None

    def evaluate_simple_expression(self, expr, var_values=None):
        """评估简单的算术表达式（支持变量）"""
        if var_values is None:
            var_values = {}

        try:
            expr = expr.strip()

            # 处理类型转换
            if expr.startswith('(int)'):
                inner = expr[5:].strip()
                val = self.evaluate_simple_expression(inner, var_values)
                return int(val) if val is not None else None

            if expr.startswith('(float)'):
                inner = expr[7:].strip()
                val = self.evaluate_simple_expression(inner, var_values)
                return float(val) if val is not None else None

            # 处理一元负号
            if expr.startswith('-'):
                inner = expr[1:].strip()
                val = self.evaluate_simple_expression(inner, var_values)
                return -val if val is not None else None

            # 如果是变量，查找其值
            if expr in var_values:
                return var_values[expr]

            # 如果是数字
            if expr.replace('.', '').isdigit():
                return float(expr) if '.' in expr else int(expr)

            # 处理加法 a + b
            if '+' in expr and expr.count('+') == 1:
                parts = expr.split('+')
                if len(parts) == 2:
                    left = self.evaluate_simple_expression(parts[0].strip(), var_values)
                    right = self.evaluate_simple_expression(parts[1].strip(), var_values)
                    if left is not None and right is not None:
                        return left + right

            # 处理减法 a - b
            if '-' in expr and expr.count('-') == 1:
                parts = expr.split('-')
                if len(parts) == 2:
                    left = self.evaluate_simple_expression(parts[0].strip(), var_values)
                    right = self.evaluate_simple_expression(parts[1].strip(), var_values)
                    if left is not None and right is not None:
                        return left - right

            # 处理乘法 a * b
            if '*' in expr and expr.count('*') == 1:
                parts = expr.split('*')
                if len(parts) == 2:
                    left = self.evaluate_simple_expression(parts[0].strip(), var_values)
                    right = self.evaluate_simple_expression(parts[1].strip(), var_values)
                    if left is not None and right is not None:
                        return left * right

            # 处理除法 a / b
            if '/' in expr and expr.count('/') == 1:
                parts = expr.split('/')
                if len(parts) == 2:
                    left = self.evaluate_simple_expression(parts[0].strip(), var_values)
                    right = self.evaluate_simple_expression(parts[1].strip(), var_values)
                    if left is not None and right is not None and right != 0:
                        return left / right

            return None
        except:
            return None

    def extract_section(self, output, section_name, target_widget):
        """从输出中提取特定章节"""
        lines = output.split('\n')
        in_section = False
        section_content = []

        for line in lines:
            if section_name in line:
                in_section = True
                continue
            if in_section:
                if line.startswith('===') or line.startswith('━'):
                    break
                section_content.append(line)

        if section_content:
            target_widget.insert(1.0, '\n'.join(section_content))

    def display_tokens(self, output):
        """显示Token"""
        self.token_output.delete(1.0, tk.END)
        self.token_output.insert(tk.END, f"{'类型':<12} {'值':<20} {'行号':<6}\n", "header")
        self.token_output.insert(tk.END, "-" * 45 + "\n", "header")

        lines = output.split('\n')
        token_count = 0

        for line in lines:
            # 去除首尾空白
            original_line = line
            line = line.strip()

            # 跳过空行、分隔线、标题行
            if not line:
                continue
            if line.startswith('=') or line.startswith('-') or line.startswith('+'):
                continue
            if 'Token 类型' in line or '内容' in line or '行号' in line:
                continue
            if '词法分析' in line or '目标' in line or '识别' in line:
                continue
            if '总计' in line:
                # 提取总数信息
                if '个Token' in line:
                    try:
                        count_str = line.split('总计')[1].split('个')[0].strip()
                        if count_str.isdigit():
                            token_count = int(count_str)
                    except:
                        pass
                continue
            if '错误' in line:
                # 显示错误信息
                self.token_output.insert(tk.END, line + "\n", "line")
                continue

            # 尝试解析表格格式: | KEYWORD | int | 1 |
            if line.startswith('|'):
                parts = [p.strip() for p in line.split('|')]
                # 去掉首尾空元素
                parts = [p for p in parts if p]

                if len(parts) >= 2:
                    token_type = parts[0]
                    token_value = parts[1] if len(parts) > 1 else ""
                    line_num = parts[2] if len(parts) > 2 else "1"

                    # 过滤掉非token行
                    if token_type and token_type not in ['Token 类型', '---']:
                        self.token_output.insert(tk.END, f"{token_type:<12} ", "type")
                        self.token_output.insert(tk.END, f"{token_value:<20} ", "value")
                        self.token_output.insert(tk.END, f"{line_num:<6}\n", "line")
                        token_count += 1

            # 尝试解析简单格式: KEYWORD int 1
            else:
                parts = line.split()
                if len(parts) >= 2:
                    if parts[0] in ['KEYWORD', 'OPERATOR', 'SYMBOL', 'NUMBER', 'FLOAT',
                                    'IDENTIFIER', 'UNKNOWN']:
                        token_type = parts[0]
                        token_value = parts[1] if len(parts) > 1 else ""
                        line_num = parts[2] if len(parts) > 2 else "1"

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
        asm_lines = []

        for line in lines:
            # 检测汇编代码开始
            if '汇编' in line or 'Assembly' in line or '.section' in line or 'global main' in line:
                in_asm = True

            # 检测汇编代码结束
            if in_asm:
                if '编译完成' in line or '目标代码已保存' in line:
                    break
                # 跳过非汇编行
                if not line.strip() or line.startswith('===') or line.startswith('━') or '目标代码' in line:
                    continue
                asm_lines.append(line)

        if asm_lines:
            self.asm_output.insert(1.0, '\n'.join(asm_lines))
        else:
            self.asm_output.insert(1.0, "(未生成汇编代码)\n")


def main():
    root = tk.Tk()
    app = SysCompilerGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()

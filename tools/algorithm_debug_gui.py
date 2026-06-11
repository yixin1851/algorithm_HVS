from __future__ import annotations

import json
import os
import sys
from pathlib import Path

try:
    from PySide6.QtCore import QProcess, QTimer, Qt
    from PySide6.QtGui import QTextCursor
    from PySide6.QtWidgets import (
        QApplication,
        QCheckBox,
        QComboBox,
        QFileDialog,
        QFormLayout,
        QFrame,
        QGridLayout,
        QGroupBox,
        QHBoxLayout,
        QLabel,
        QLineEdit,
        QListWidget,
        QListWidgetItem,
        QMainWindow,
        QMessageBox,
        QPushButton,
        QPlainTextEdit,
        QSpinBox,
        QSplitter,
        QTabWidget,
        QVBoxLayout,
        QWidget,
    )
except ImportError as exc:
    print("PySide6 is not installed. Install it with: python -m pip install PySide6")
    raise SystemExit(1) from exc


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DEMO = ROOT / "BUILD" / "ReleaseAlgorithmDemo" / "x64" / "bin" / "AlgorithmDemod.exe"
DEFAULT_PROFILE = ROOT / "AlgorithmDemo" / "config" / "aps_gui_profile.json"
LAST_SESSION = ROOT / "AlgorithmDemo" / "config" / "debug_gui_session.json"


BREAKPOINT_PRESETS = (
    ("main 入口", "main.cpp", 19),
    ("profile 分支", "main.cpp", 26),
    ("RunApsRawTestProfile", "ApsRawTestRunner.cpp", 1309),
    ("LoadProfileConfig", "ApsRawTestRunner.cpp", 594),
    ("RunSelectedTests", "ApsRawTestRunner.cpp", 951),
    ("CreateAPSAlgoInterface", "ApsRawTestRunner.cpp", 971),
    ("ImportRawData", "ApsRawTestRunner.cpp", 1019),
    ("Demo SNoise 调用", "ApsRawTestRunner.cpp", 1035),
    ("Demo TNoise 调用", "ApsRawTestRunner.cpp", 1052),
    ("Demo BadPixel 调用", "ApsRawTestRunner.cpp", 1063),
    ("DLL TNoise 实现", "AlpAPSMPAlgorithm.cpp", 124),
    ("DLL SNoise 实现", "AlpAPSMPAlgorithm.cpp", 216),
    ("DLL BadPixel 实现", "AlpAPSMPAlgorithm.cpp", 310),
    ("003CA BadPixel 实现", "Alp003CAAPSMPAlgorithm.cpp", 171),
)


FUNCTION_BREAKPOINT_PRESETS = (
    "RunApsRawTestProfile",
    "RunSelectedTests",
    "LoadProfileConfig",
    "CreateAPSAlgoInterface",
    "CAlpAPSMPAlgorithm::SNoise",
    "CAlpAPSMPAlgorithm::TNoise",
    "CAlpAPSMPAlgorithm::BadPixel",
    "CAlpAPSMPAlgorithm::HotPixel",
    "CAlpAPSMPAlgorithm::DataMean",
    "CAlpAPSMPAlgorithm::DSNU",
    "CAlp003CAAPSMPAlgorithm::BadPixel",
)


MEMORY_FORMATS = (
    ("hex", "x"),
    ("decimal", "d"),
    ("unsigned", "u"),
    ("char", "c"),
    ("float", "f"),
)


def discover_lldb() -> Path | None:
    env_lldb = os.environ.get("LLDB_EXE")
    if env_lldb and Path(env_lldb).exists():
        return Path(env_lldb)

    clion_home = os.environ.get("CLION_HOME")
    if clion_home:
        candidate = Path(clion_home) / "bin" / "lldb" / "win" / "x64" / "bin" / "lldb.exe"
        if candidate.exists():
            return candidate

    roots = [
        Path("D:/Program Files/JetBrains"),
        Path("C:/Program Files/JetBrains"),
        Path(os.environ.get("ProgramFiles", "C:/Program Files")) / "JetBrains",
    ]
    seen: set[Path] = set()
    for root in roots:
        if root in seen or not root.exists():
            continue
        seen.add(root)
        for clion_dir in sorted(root.glob("CLion*"), reverse=True):
            candidate = clion_dir / "bin" / "lldb" / "win" / "x64" / "bin" / "lldb.exe"
            if candidate.exists():
                return candidate
    return None


def quote_lldb_arg(value: str) -> str:
    normalized = str(Path(value)).replace("\\", "/")
    return '"' + normalized.replace('"', '\\"') + '"'


class AlgorithmDebugWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("Algorithm LLDB Debug GUI")
        self.process: QProcess | None = None
        self.stop_requested = False
        self.target_state = "idle"

        central = QWidget()
        self.setCentralWidget(central)
        root_layout = QVBoxLayout(central)
        root_layout.setContentsMargins(10, 10, 10, 10)

        root_layout.addWidget(self.build_path_panel())

        splitter = QSplitter(Qt.Orientation.Horizontal)
        splitter.addWidget(self.build_left_panel())
        splitter.addWidget(self.build_output_panel())
        splitter.setSizes([420, 760])
        root_layout.addWidget(splitter, 1)

        self.statusBar().showMessage("Idle")
        self.load_session()
        self.refresh_control_state()
        self.resize(1220, 760)

    def build_path_panel(self) -> QWidget:
        box = QGroupBox("调试配置")
        layout = QFormLayout(box)

        self.lldb_path = QLineEdit(str(discover_lldb() or ""))
        self.demo_path = QLineEdit(str(DEFAULT_DEMO))
        self.profile_path = QLineEdit(str(DEFAULT_PROFILE))
        self.auto_main = QCheckBox("启动后停在 main")
        self.auto_main.setChecked(True)
        self.main_line = self.spin(1, 100000, 19)
        self.clear_output_on_launch = QCheckBox("启动前清空输出")
        self.clear_output_on_launch.setChecked(True)

        layout.addRow("LLDB", self.path_row(self.lldb_path, self.browse_lldb))
        layout.addRow("Demo exe", self.path_row(self.demo_path, self.browse_demo))
        layout.addRow("Profile", self.path_row(self.profile_path, self.browse_profile))
        layout.addRow("Main 断点", self.inline_row(self.auto_main, QLabel("line"), self.main_line))
        layout.addRow("输出", self.clear_output_on_launch)
        return box

    def build_left_panel(self) -> QWidget:
        panel = QWidget()
        layout = QVBoxLayout(panel)

        layout.addWidget(self.build_launch_panel())
        layout.addWidget(self.build_breakpoint_panel(), 1)
        layout.addWidget(self.build_command_panel())
        return panel

    def build_launch_panel(self) -> QWidget:
        box = QGroupBox("调试控制")
        layout = QGridLayout(box)

        self.launch_button = QPushButton("启动")
        self.continue_button = QPushButton("继续")
        self.next_button = QPushButton("单步跳过")
        self.step_button = QPushButton("单步进入")
        self.finish_button = QPushButton("跳出函数")
        self.backtrace_button = QPushButton("调用栈")
        self.variables_button = QPushButton("变量")
        self.stop_button = QPushButton("停止")

        self.launch_button.clicked.connect(self.launch_debugger)
        self.continue_button.clicked.connect(lambda: self.send_target_command("continue"))
        self.next_button.clicked.connect(lambda: self.send_target_command("next"))
        self.step_button.clicked.connect(lambda: self.send_target_command("step"))
        self.finish_button.clicked.connect(lambda: self.send_target_command("finish"))
        self.backtrace_button.clicked.connect(lambda: self.send_target_command("bt", resumes_process=False))
        self.variables_button.clicked.connect(lambda: self.send_target_command("frame variable", resumes_process=False))
        self.stop_button.clicked.connect(self.stop_debugger)

        layout.addWidget(self.launch_button, 0, 0)
        layout.addWidget(self.continue_button, 0, 1)
        layout.addWidget(self.next_button, 1, 0)
        layout.addWidget(self.step_button, 1, 1)
        layout.addWidget(self.finish_button, 2, 0)
        layout.addWidget(self.backtrace_button, 2, 1)
        layout.addWidget(self.variables_button, 3, 0)
        layout.addWidget(self.stop_button, 3, 1)
        return box

    def build_breakpoint_panel(self) -> QWidget:
        box = QGroupBox("断点")
        layout = QVBoxLayout(box)

        self.preset_combo = QComboBox()
        for label, file_name, line in BREAKPOINT_PRESETS:
            self.preset_combo.addItem(label, (label, file_name, line))
        self.preset_combo.currentIndexChanged.connect(self.apply_selected_preset)
        layout.addWidget(self.preset_combo)

        self.break_file = QLineEdit()
        self.break_line = self.spin(1, 100000, 17)
        layout.addWidget(self.form_row("File", self.break_file))
        layout.addWidget(self.form_row("Line", self.break_line))

        button_row = QHBoxLayout()
        self.add_break_button = QPushButton("添加断点")
        self.run_to_break_button = QPushButton("运行到选中断点")
        self.remove_break_button = QPushButton("删除选中")
        self.clear_break_button = QPushButton("清空")
        self.list_break_button = QPushButton("列出")
        self.add_break_button.clicked.connect(self.add_breakpoint)
        self.run_to_break_button.clicked.connect(self.run_to_selected_breakpoint)
        self.remove_break_button.clicked.connect(self.remove_selected_breakpoint)
        self.clear_break_button.clicked.connect(self.clear_breakpoints)
        self.list_break_button.clicked.connect(lambda: self.send_command("breakpoint list"))
        button_row.addWidget(self.add_break_button)
        button_row.addWidget(self.run_to_break_button)
        button_row.addWidget(self.remove_break_button)
        button_row.addWidget(self.clear_break_button)
        button_row.addWidget(self.list_break_button)
        layout.addLayout(button_row)

        self.break_list = QListWidget()
        layout.addWidget(self.break_list, 1)
        self.apply_selected_preset()
        return box

    def build_command_panel(self) -> QWidget:
        box = QGroupBox("LLDB 命令")
        layout = QVBoxLayout(box)

        tabs = QTabWidget()

        command_page = QWidget()
        command_layout = QVBoxLayout(command_page)
        self.command_input = QLineEdit()
        self.command_input.setPlaceholderText("例如：b ApsRawTestRunner.cpp:1035")
        self.command_input.returnPressed.connect(self.send_manual_command)
        self.send_button = QPushButton("发送")
        self.send_button.clicked.connect(self.send_manual_command)
        command_layout.addWidget(self.inline_row(self.command_input, self.send_button))

        self.variable_input = QLineEdit()
        self.variable_input.setPlaceholderText("变量名，例如：config / result / ok")
        self.variable_value_button = QPushButton("查看值")
        self.variable_address_button = QPushButton("查看地址")
        self.variable_value_button.clicked.connect(self.show_variable_value)
        self.variable_address_button.clicked.connect(self.show_variable_address)
        command_layout.addWidget(self.inline_row(self.variable_input, self.variable_value_button, self.variable_address_button))
        tabs.addTab(command_page, "命令")

        function_page = QWidget()
        function_layout = QVBoxLayout(function_page)
        self.function_combo = QComboBox()
        self.function_combo.addItems(FUNCTION_BREAKPOINT_PRESETS)
        self.function_name = QLineEdit(FUNCTION_BREAKPOINT_PRESETS[0])
        self.function_combo.currentTextChanged.connect(self.function_name.setText)
        self.add_function_break_button = QPushButton("添加函数断点")
        self.run_to_function_button = QPushButton("运行到函数")
        self.add_function_break_button.clicked.connect(self.add_function_breakpoint)
        self.run_to_function_button.clicked.connect(self.run_to_function)
        function_layout.addWidget(self.form_row("Preset", self.function_combo))
        function_layout.addWidget(self.form_row("Name", self.function_name))
        function_layout.addWidget(self.inline_row(self.add_function_break_button, self.run_to_function_button))
        tabs.addTab(function_page, "函数")

        thread_page = QWidget()
        thread_layout = QVBoxLayout(thread_page)
        self.thread_index = self.spin(1, 10000, 1)
        self.thread_list_button = QPushButton("线程列表")
        self.thread_select_button = QPushButton("选择线程")
        self.thread_bt_button = QPushButton("当前线程栈")
        self.thread_bt_all_button = QPushButton("所有线程栈")
        self.thread_list_button.clicked.connect(lambda: self.send_target_command("thread list", resumes_process=False))
        self.thread_select_button.clicked.connect(self.select_thread)
        self.thread_bt_button.clicked.connect(lambda: self.send_target_command("thread backtrace", resumes_process=False))
        self.thread_bt_all_button.clicked.connect(lambda: self.send_target_command("thread backtrace all", resumes_process=False))
        thread_layout.addWidget(self.form_row("Thread #", self.thread_index))
        thread_layout.addWidget(self.inline_row(self.thread_list_button, self.thread_select_button))
        thread_layout.addWidget(self.inline_row(self.thread_bt_button, self.thread_bt_all_button))
        tabs.addTab(thread_page, "线程")

        memory_page = QWidget()
        memory_layout = QVBoxLayout(memory_page)
        self.memory_expr = QLineEdit("&config")
        self.memory_expr.setPlaceholderText("地址或表达式，例如：0x1234 / &config / raw.data() / aps")
        self.memory_format = QComboBox()
        for label, fmt in MEMORY_FORMATS:
            self.memory_format.addItem(label, fmt)
        self.memory_size = self.spin(1, 8, 1)
        self.memory_count = self.spin(1, 4096, 64)
        self.memory_read_button = QPushButton("读取内存")
        self.memory_read_var_button = QPushButton("读变量内存")
        self.memory_read_pointer_button = QPushButton("读指针内容")
        self.memory_read_button.clicked.connect(self.read_memory)
        self.memory_read_var_button.clicked.connect(self.read_variable_memory)
        self.memory_read_pointer_button.clicked.connect(self.read_pointer_memory)
        memory_layout.addWidget(self.form_row("Expr", self.memory_expr))
        memory_layout.addWidget(self.inline_row(QLabel("Format"), self.memory_format, QLabel("Size"), self.memory_size, QLabel("Count"), self.memory_count))
        memory_layout.addWidget(self.inline_row(self.memory_read_button, self.memory_read_var_button, self.memory_read_pointer_button))
        tabs.addTab(memory_page, "内存")

        layout.addWidget(tabs)
        return box

    def build_output_panel(self) -> QWidget:
        panel = QWidget()
        layout = QVBoxLayout(panel)

        self.output = QPlainTextEdit()
        self.output.setReadOnly(True)

        toolbar = QFrame()
        toolbar_layout = QHBoxLayout(toolbar)
        toolbar_layout.setContentsMargins(0, 0, 0, 0)
        clear_button = QPushButton("清空输出")
        clear_button.clicked.connect(self.output.clear)
        help_button = QPushButton("常用命令")
        help_button.clicked.connect(self.show_command_help)
        toolbar_layout.addWidget(QLabel("LLDB 输出"))
        toolbar_layout.addStretch(1)
        toolbar_layout.addWidget(clear_button)
        toolbar_layout.addWidget(help_button)
        layout.addWidget(toolbar)
        layout.addWidget(self.output, 1)
        return panel

    def spin(self, minimum: int, maximum: int, value: int) -> QSpinBox:
        widget = QSpinBox()
        widget.setRange(minimum, maximum)
        widget.setValue(value)
        return widget

    def path_row(self, edit: QLineEdit, slot) -> QWidget:
        browse = QPushButton("...")
        browse.setFixedWidth(36)
        browse.clicked.connect(slot)
        return self.inline_row(edit, browse)

    def inline_row(self, *widgets: QWidget) -> QWidget:
        row = QWidget()
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 0, 0, 0)
        for widget in widgets:
            layout.addWidget(widget)
        return row

    def form_row(self, label: str, widget: QWidget) -> QWidget:
        row = QWidget()
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(QLabel(label))
        layout.addWidget(widget, 1)
        return row

    def browse_lldb(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Select LLDB", self.lldb_path.text() or str(ROOT), "LLDB (*.exe)")
        if path:
            self.lldb_path.setText(path)

    def browse_demo(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Select AlgorithmDemo", self.demo_path.text(), "Executable (*.exe)")
        if path:
            self.demo_path.setText(path)

    def browse_profile(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Select profile", self.profile_path.text(), "JSON (*.json)")
        if path:
            self.profile_path.setText(path)

    def apply_selected_preset(self) -> None:
        data = self.preset_combo.currentData()
        if data is None:
            return
        _, file_name, line = data
        self.break_file.setText(file_name)
        self.break_line.setValue(line)

    def add_breakpoint(self) -> None:
        file_name = self.break_file.text().strip()
        if not file_name:
            QMessageBox.warning(self, "断点无效", "请填写文件名。")
            return

        line = self.break_line.value()
        label = self.current_breakpoint_label(file_name, line)
        command = f"breakpoint set --file {file_name} --line {line}"

        item = QListWidgetItem(label)
        item.setData(Qt.ItemDataRole.UserRole, command)
        self.break_list.addItem(item)

        if self.is_running() and self.target_state == "stopped":
            self.send_command(command)
        elif self.is_running():
            self.append_output(
                "[pending breakpoint] target is running; this breakpoint will be applied on next launch.\n"
            )
        else:
            self.append_output(f"[pending breakpoint] {command}\n")

    def run_to_selected_breakpoint(self) -> None:
        file_name = self.break_file.text().strip()
        if not file_name:
            QMessageBox.warning(self, "断点无效", "请先选择预设断点或填写文件名。")
            return
        self.run_to_location(file_name, self.break_line.value(), self.current_breakpoint_label(file_name, self.break_line.value()))

    def add_function_breakpoint(self) -> None:
        name = self.function_name.text().strip()
        if not name:
            QMessageBox.warning(self, "断点无效", "请填写函数名。")
            return

        command = f"breakpoint set --name {name}"
        item = QListWidgetItem(f"function: {name}")
        item.setData(Qt.ItemDataRole.UserRole, command)
        self.break_list.addItem(item)

        if self.is_running() and self.target_state == "stopped":
            self.send_command(command)
        elif self.is_running():
            self.append_output(
                "[pending breakpoint] target is running; this function breakpoint will be applied on next launch.\n"
            )
        else:
            self.append_output(f"[pending breakpoint] {command}\n")

    def run_to_function(self) -> None:
        name = self.function_name.text().strip()
        if not name:
            QMessageBox.warning(self, "断点无效", "请填写函数名。")
            return
        self.run_to_function_name(name)

    def current_breakpoint_label(self, file_name: str, line: int) -> str:
        preset = self.preset_combo.currentData()
        if preset is not None and preset[1] == file_name and preset[2] == line:
            return f"{file_name}:{line}  {preset[0]}"
        return f"{file_name}:{line}"

    def remove_selected_breakpoint(self) -> None:
        row = self.break_list.currentRow()
        if row >= 0:
            self.break_list.takeItem(row)
            self.append_output("[note] GUI list item removed. Use breakpoint list/delete in LLDB for already-set breakpoints.\n")

    def clear_breakpoints(self) -> None:
        self.break_list.clear()
        if self.is_running():
            self.send_command("breakpoint delete")

    def launch_debugger(self) -> None:
        if self.is_running():
            QMessageBox.information(self, "已经启动", "LLDB 已经在运行。")
            return

        lldb = Path(self.lldb_path.text())
        demo = Path(self.demo_path.text())
        profile = Path(self.profile_path.text())
        if not lldb.exists():
            QMessageBox.warning(self, "Missing LLDB", f"LLDB 不存在：\n{lldb}")
            return
        if not demo.exists():
            QMessageBox.warning(self, "Missing executable", f"Demo exe 不存在：\n{demo}")
            return
        if not profile.exists():
            QMessageBox.warning(self, "Missing profile", f"Profile 不存在：\n{profile}")
            return

        self.save_session()
        if self.clear_output_on_launch.isChecked():
            self.output.clear()
        self.append_output("Launching LLDB for AlgorithmDemo...\n")
        self.append_output(f"  lldb:    {lldb}\n")
        self.append_output(f"  exe:     {demo}\n")
        self.append_output(f"  profile: {profile}\n\n")

        self.process = QProcess(self)
        self.process.setProgram(str(lldb))
        self.process.setArguments([str(demo)])
        self.process.setWorkingDirectory(str(ROOT))
        self.process.readyReadStandardOutput.connect(self.read_stdout)
        self.process.readyReadStandardError.connect(self.read_stderr)
        self.process.started.connect(self.on_started)
        self.process.finished.connect(self.on_finished)
        self.stop_requested = False
        self.target_state = "launching"
        self.process.start()
        self.statusBar().showMessage("Starting LLDB")
        self.refresh_control_state()

    def on_started(self) -> None:
        self.statusBar().showMessage("LLDB started")
        self.send_command("settings set auto-confirm true", echo=True)

        commands = []
        if self.auto_main.isChecked():
            commands.append(f"breakpoint set --file main.cpp --line {self.main_line.value()}")
        commands.extend(self.breakpoint_commands())
        commands.append(f"process launch -- --profile {quote_lldb_arg(self.profile_path.text())}")

        for command in commands:
            self.send_command(command, echo=True)

    def breakpoint_commands(self) -> list[str]:
        commands = []
        seen = set()
        for index in range(self.break_list.count()):
            command = self.break_list.item(index).data(Qt.ItemDataRole.UserRole)
            if command and command not in seen:
                commands.append(command)
                seen.add(command)
        return commands

    def send_manual_command(self) -> None:
        command = self.command_input.text().strip()
        if not command:
            return
        self.command_input.clear()
        if self.command_requires_stopped_target(command):
            self.send_target_command(command)
            return
        self.send_command(command)

    def show_variable_value(self) -> None:
        name = self.variable_input.text().strip()
        if not name:
            QMessageBox.warning(self, "变量为空", "请先输入变量名。")
            return
        self.send_target_command(f"frame variable {name}", resumes_process=False)

    def show_variable_address(self) -> None:
        name = self.variable_input.text().strip()
        if not name:
            QMessageBox.warning(self, "变量为空", "请先输入变量名。")
            return
        self.send_target_command(f"expression -- &{name}", resumes_process=False)

    def select_thread(self) -> None:
        self.send_target_command(f"thread select {self.thread_index.value()}", resumes_process=False)

    def read_memory(self) -> None:
        expr = self.memory_expr.text().strip()
        if not expr:
            QMessageBox.warning(self, "地址为空", "请先输入地址或表达式。")
            return
        self.send_target_command(self.memory_read_command(expr), resumes_process=False)

    def read_variable_memory(self) -> None:
        name = self.variable_input.text().strip()
        if not name:
            QMessageBox.warning(self, "变量为空", "请先输入变量名。")
            return
        self.memory_expr.setText(f"&{name}")
        self.send_target_command(self.memory_read_command(f"&{name}"), resumes_process=False)

    def read_pointer_memory(self) -> None:
        name = self.variable_input.text().strip()
        if not name:
            QMessageBox.warning(self, "变量为空", "请先输入变量名。")
            return
        self.memory_expr.setText(name)
        self.send_target_command(self.memory_read_command(name), resumes_process=False)

    def memory_read_command(self, expr: str) -> str:
        return (
            "memory read "
            f"--format {self.memory_format.currentData()} "
            f"--size {self.memory_size.value()} "
            f"--count {self.memory_count.value()} "
            f"-- {expr}"
        )

    def send_target_command(self, command: str, resumes_process: bool = True) -> None:
        if not self.is_running():
            QMessageBox.warning(self, "LLDB 未启动", "请先点击“启动”。")
            return
        if self.target_state != "stopped":
            QMessageBox.information(
                self,
                "进程尚未停住",
                "请等待 LLDB 输出 Process stopped 后再单步，或先设置断点再继续。",
            )
            return
        self.send_command(command)
        if resumes_process:
            self.target_state = "running"
            self.statusBar().showMessage("Process running")
            self.refresh_control_state()

    def run_to_location(self, file_name: str, line: int, label: str) -> None:
        if not self.is_running():
            QMessageBox.warning(self, "LLDB 未启动", "请先点击“启动”。")
            return
        if self.target_state != "stopped":
            QMessageBox.information(
                self,
                "进程尚未停住",
                "请等待 LLDB 输出 Process stopped 后再使用该按钮。",
            )
            return

        self.append_output(f"(gui) run to {label}: {file_name}:{line}\n")
        self.send_command(f"breakpoint set --file {file_name} --line {line}", echo=False)
        self.send_command("continue", echo=False)
        self.target_state = "running"
        self.statusBar().showMessage(f"Running to {label}")
        self.refresh_control_state()

    def run_to_function_name(self, name: str) -> None:
        if not self.is_running():
            QMessageBox.warning(self, "LLDB 未启动", "请先点击“启动”。")
            return
        if self.target_state != "stopped":
            QMessageBox.information(
                self,
                "进程尚未停住",
                "请等待 LLDB 输出 Process stopped 后再使用该按钮。",
            )
            return

        self.append_output(f"(gui) run to function: {name}\n")
        self.send_command(f"breakpoint set --name {name}", echo=False)
        self.send_command("continue", echo=False)
        self.target_state = "running"
        self.statusBar().showMessage(f"Running to {name}")
        self.refresh_control_state()

    @staticmethod
    def command_requires_stopped_target(command: str) -> bool:
        first = command.strip().split(maxsplit=1)[0].lower()
        return first in {
            "c",
            "continue",
            "n",
            "next",
            "s",
            "step",
            "finish",
            "bt",
            "frame",
            "thread",
            "memory",
        }

    def send_command(self, command: str, echo: bool = True) -> None:
        if not self.is_running():
            QMessageBox.warning(self, "LLDB 未启动", "请先点击“启动”。")
            return
        if echo:
            self.append_output(f"(gui) {command}\n")
        assert self.process is not None
        self.process.write((command + "\n").encode("utf-8"))

    def stop_debugger(self) -> None:
        if not self.is_running():
            return
        reply = QMessageBox.question(
            self,
            "停止调试",
            "是否停止当前 LLDB 调试会话？",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if reply != QMessageBox.StandardButton.Yes:
            return
        self.stop_requested = True
        self.target_state = "stopping"
        self.statusBar().showMessage("Stopping LLDB")
        self.append_output("(gui) process kill\n(gui) quit\n")
        assert self.process is not None
        self.process.write(b"process kill\nquit\n")
        QTimer.singleShot(3000, self.kill_if_running)

    def kill_if_running(self) -> None:
        if self.process is not None and self.process.state() != QProcess.ProcessState.NotRunning:
            self.process.kill()

    def read_stdout(self) -> None:
        if self.process is None:
            return
        text = bytes(self.process.readAllStandardOutput()).decode(errors="replace")
        self.append_output(text)

    def read_stderr(self) -> None:
        if self.process is None:
            return
        text = bytes(self.process.readAllStandardError()).decode(errors="replace")
        self.append_output(text)

    def append_output(self, text: str) -> None:
        if not text:
            return
        cursor = self.output.textCursor()
        cursor.movePosition(QTextCursor.MoveOperation.End)
        cursor.insertText(text)
        self.output.setTextCursor(cursor)
        self.output.ensureCursorVisible()
        self.update_status_from_output(text)

    def update_status_from_output(self, text: str) -> None:
        lower = text.lower()
        if "process " in lower and " launched:" in lower and self.target_state != "stopped":
            self.target_state = "running"
            self.statusBar().showMessage("Process running")
        if "process stopped" in lower or "stop reason =" in lower:
            self.target_state = "stopped"
            self.statusBar().showMessage("Process stopped")
        elif "process exited" in lower:
            self.target_state = "exited"
            self.statusBar().showMessage("Process exited")
        elif "error:" in lower:
            self.statusBar().showMessage("LLDB reported an error")
        self.refresh_control_state()

    def on_finished(self, exit_code: int = 0, exit_status=None) -> None:
        self.append_output(f"\nLLDB finished. exit={exit_code}\n")
        if self.stop_requested:
            self.statusBar().showMessage("Stopped")
        else:
            self.statusBar().showMessage(f"LLDB exited: {exit_code}")
        self.process = None
        self.stop_requested = False
        self.target_state = "idle"
        self.refresh_control_state()

    def is_running(self) -> bool:
        return self.process is not None and self.process.state() != QProcess.ProcessState.NotRunning

    def refresh_control_state(self) -> None:
        lldb_running = self.is_running()
        target_stopped = lldb_running and self.target_state == "stopped"
        self.launch_button.setEnabled(not lldb_running)
        for button in (
            self.continue_button,
            self.next_button,
            self.step_button,
            self.finish_button,
            self.backtrace_button,
            self.variables_button,
            self.run_to_break_button,
            self.run_to_function_button,
            self.thread_list_button,
            self.thread_select_button,
            self.thread_bt_button,
            self.thread_bt_all_button,
            self.memory_read_button,
            self.memory_read_var_button,
            self.memory_read_pointer_button,
            self.variable_value_button,
            self.variable_address_button,
        ):
            button.setEnabled(target_stopped)
        for button in (
            self.stop_button,
            self.list_break_button,
            self.send_button,
        ):
            button.setEnabled(lldb_running)
        self.add_function_break_button.setEnabled(True)
        self.command_input.setEnabled(lldb_running)
        self.variable_input.setEnabled(lldb_running)
        self.function_combo.setEnabled(True)
        self.function_name.setEnabled(True)
        self.thread_index.setEnabled(lldb_running)
        self.memory_expr.setEnabled(lldb_running)
        self.memory_format.setEnabled(lldb_running)
        self.memory_size.setEnabled(lldb_running)
        self.memory_count.setEnabled(lldb_running)

    def show_command_help(self) -> None:
        QMessageBox.information(
            self,
            "常用 LLDB 命令",
            "\n".join(
                [
                    "b file:line              设置断点",
                    "breakpoint list          查看断点",
                    "breakpoint delete 5      删除断点",
                    "continue                 继续运行",
                    "next                     单步跳过",
                    "step                     单步进入",
                    "finish                   跳出函数",
                    "bt                       查看调用栈",
                    "frame variable           查看变量",
                    "breakpoint set --name f  按函数名设置断点",
                    "thread list              查看线程",
                    "thread select N          切换线程",
                    "thread backtrace all     查看所有线程调用栈",
                    "memory read -- &var      查看变量地址内存",
                    "quit                     退出 LLDB",
                ]
            ),
        )

    def save_session(self) -> None:
        data = {
            "lldb_path": self.lldb_path.text(),
            "demo_path": self.demo_path.text(),
            "profile_path": self.profile_path.text(),
            "auto_main": self.auto_main.isChecked(),
            "main_line": self.main_line.value(),
            "clear_output_on_launch": self.clear_output_on_launch.isChecked(),
            "function_name": self.function_name.text(),
            "thread_index": self.thread_index.value(),
            "memory_expr": self.memory_expr.text(),
            "memory_format": self.memory_format.currentText(),
            "memory_size": self.memory_size.value(),
            "memory_count": self.memory_count.value(),
            "breakpoints": [
                {
                    "text": self.break_list.item(index).text(),
                    "command": self.break_list.item(index).data(Qt.ItemDataRole.UserRole),
                }
                for index in range(self.break_list.count())
            ],
        }
        LAST_SESSION.parent.mkdir(parents=True, exist_ok=True)
        LAST_SESSION.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")

    def load_session(self) -> None:
        if not LAST_SESSION.exists():
            return
        try:
            data = json.loads(LAST_SESSION.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            return

        self.lldb_path.setText(str(data.get("lldb_path") or self.lldb_path.text()))
        self.demo_path.setText(str(data.get("demo_path") or self.demo_path.text()))
        self.profile_path.setText(str(data.get("profile_path") or self.profile_path.text()))
        self.auto_main.setChecked(bool(data.get("auto_main", self.auto_main.isChecked())))
        self.main_line.setValue(int(data.get("main_line", self.main_line.value())))
        self.clear_output_on_launch.setChecked(
            bool(data.get("clear_output_on_launch", self.clear_output_on_launch.isChecked()))
        )
        self.function_name.setText(str(data.get("function_name") or self.function_name.text()))
        self.thread_index.setValue(int(data.get("thread_index", self.thread_index.value())))
        self.memory_expr.setText(str(data.get("memory_expr") or self.memory_expr.text()))
        self.set_combo(self.memory_format, data.get("memory_format"))
        self.memory_size.setValue(int(data.get("memory_size", self.memory_size.value())))
        self.memory_count.setValue(int(data.get("memory_count", self.memory_count.value())))

        self.break_list.clear()
        for breakpoint in data.get("breakpoints", []):
            command = breakpoint.get("command")
            text = breakpoint.get("text") or command
            if not command or not text:
                continue
            item = QListWidgetItem(text)
            item.setData(Qt.ItemDataRole.UserRole, command)
            self.break_list.addItem(item)

    def set_combo(self, combo: QComboBox, value: str | None) -> None:
        if value is None:
            return
        index = combo.findText(value)
        if index >= 0:
            combo.setCurrentIndex(index)

    def closeEvent(self, event) -> None:
        self.save_session()
        if self.is_running():
            reply = QMessageBox.question(
                self,
                "退出调试 GUI",
                "LLDB 仍在运行，是否退出并停止调试？",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No,
            )
            if reply != QMessageBox.StandardButton.Yes:
                event.ignore()
                return
            assert self.process is not None
            self.process.write(b"settings set auto-confirm true\nprocess kill\nquit\n")
            QTimer.singleShot(1000, self.kill_if_running)
        event.accept()


def main() -> int:
    app = QApplication(sys.argv)
    window = AlgorithmDebugWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())

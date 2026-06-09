from __future__ import annotations

import json
import re
import sys
from datetime import datetime
from pathlib import Path

try:
    from PySide6.QtCore import QProcess, QTimer
    from PySide6.QtWidgets import (
        QApplication,
        QCheckBox,
        QComboBox,
        QDoubleSpinBox,
        QFileDialog,
        QFormLayout,
        QFrame,
        QGridLayout,
        QGroupBox,
        QHBoxLayout,
        QLabel,
        QLineEdit,
        QMainWindow,
        QMessageBox,
        QPushButton,
        QPlainTextEdit,
        QScrollArea,
        QSpinBox,
        QStackedWidget,
        QTabWidget,
        QVBoxLayout,
        QWidget,
    )
except ImportError as exc:
    print("PySide6 is not installed. Install it with: python -m pip install PySide6")
    raise SystemExit(1) from exc


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_DEMO = ROOT / "BUILD" / "ReleaseAlgorithmDemo" / "x64" / "bin" / "AlgorithmDemod.exe"
DEFAULT_RAW = (
    ROOT
    / "test_rawdata"
    / "testdata_center_crop_3264x2448_5frames_u16_from_3280x2464.raw"
)
DEFAULT_PROFILE = ROOT / "AlgorithmDemo" / "config" / "aps_gui_profile.json"
LAST_SESSION = ROOT / "AlgorithmDemo" / "config" / "last_session.json"
DEFAULT_OUTPUT = ROOT / "output" / "aps_gui_run"


APS_TESTS = ("snoise", "tnoise", "badpixel", "hotpixel", "datamean", "dsnu")
EVS_TESTS = (
    "count_events",
    "stationary_noise",
    "stationary_uniformity",
    "hotpixel",
    "find_peak",
    "badpixel",
)


class AlgorithmTestWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("Algorithm Debug Test")
        self.process: QProcess | None = None
        self.running = False
        self.stop_requested = False
        self.aps_checks: dict[str, QCheckBox] = {}
        self.evs_checks: dict[str, QCheckBox] = {}
        self.nav_buttons: list[QPushButton] = []

        central = QWidget()
        self.setCentralWidget(central)
        root = QHBoxLayout(central)
        root.setContentsMargins(10, 10, 10, 10)

        root.addWidget(self.build_navigation())
        root.addWidget(self.build_workspace(), 1)

        self.statusBar().showMessage("Idle")
        self.show_page(0)
        self.load_last_session()
        self.resize(1180, 760)

    def build_navigation(self) -> QWidget:
        panel = QFrame()
        panel.setFrameShape(QFrame.StyledPanel)
        panel.setFixedWidth(170)
        layout = QVBoxLayout(panel)

        for index, title in enumerate(("基本配置", "APS测试项", "EVS测试项", "测试条件", "设置")):
            button = QPushButton(title)
            button.setCheckable(True)
            button.setMinimumHeight(54)
            button.clicked.connect(lambda checked=False, i=index: self.show_page(i))
            self.nav_buttons.append(button)
            layout.addWidget(button)

        layout.addStretch(1)
        self.run_button = QPushButton("开始")
        self.run_button.setMinimumHeight(56)
        self.run_button.clicked.connect(self.toggle_run)
        layout.addWidget(self.run_button)
        return panel

    def build_workspace(self) -> QWidget:
        self.tabs = QTabWidget()
        self.stack = QStackedWidget()
        self.stack.addWidget(self.wrap_page(self.build_basic_page()))
        self.stack.addWidget(self.wrap_page(self.build_aps_page()))
        self.stack.addWidget(self.wrap_page(self.build_evs_page()))
        self.stack.addWidget(self.wrap_page(self.build_conditions_page()))
        self.stack.addWidget(self.wrap_page(self.build_settings_page()))

        config_tab = QWidget()
        config_layout = QVBoxLayout(config_tab)
        config_layout.addWidget(self.stack)

        self.log_output = QPlainTextEdit()
        self.log_output.setReadOnly(True)

        self.tabs.addTab(config_tab, "配置")
        self.tabs.addTab(self.log_output, "日志")
        return self.tabs

    def wrap_page(self, widget: QWidget) -> QWidget:
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setWidget(widget)
        return scroll

    def build_basic_page(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        path_box = QGroupBox("路径")
        path_layout = QFormLayout(path_box)
        self.demo_path = QLineEdit(str(DEFAULT_DEMO))
        self.profile_path = QLineEdit(str(DEFAULT_PROFILE))
        self.raw_path = QLineEdit(str(DEFAULT_RAW))
        self.output_dir = QLineEdit(str(DEFAULT_OUTPUT))
        self.result_json = QLineEdit("result.json")
        path_layout.addRow("Demo exe", self.path_row(self.demo_path, self.browse_demo))
        path_layout.addRow("Profile", self.path_row(self.profile_path, self.browse_profile))
        path_layout.addRow("Raw", self.path_row(self.raw_path, self.browse_raw))
        path_layout.addRow("Output", self.path_row(self.output_dir, self.browse_output))
        path_layout.addRow("Result JSON", self.result_json)
        layout.addWidget(path_box)

        sensor_box = QGroupBox("传感器")
        sensor_layout = QFormLayout(sensor_box)
        self.mode = QComboBox()
        self.mode.addItems(["APS", "EVS"])
        self.sensor = QComboBox()
        self.sensor.addItems(["ALP_003CA", "ALP_003AA", "ALP_003BA", "ALP_003BB", "ALP_004AB", "ALP_014AA", "ALP_014BA"])
        self.raw_type = QComboBox()
        self.raw_type.addItems(["UNPACK10", "UNPACK12", "RAW8", "RAW10", "RAW12"])
        self.pixel_format = QComboBox()
        self.pixel_format.addItems([
            "QuadBayerGBRG",
            "QuadBayerBGGR",
            "QuadBayerRGGB",
            "QuadBayerGRBG",
            "BayerGBRG",
            "BayerBGGR",
            "BayerRGGB",
            "BayerGRBG",
        ])
        sensor_layout.addRow("Mode", self.mode)
        sensor_layout.addRow("Sensor", self.sensor)
        sensor_layout.addRow("Raw type", self.raw_type)
        sensor_layout.addRow("Pixel format", self.pixel_format)
        layout.addWidget(sensor_box)

        raw_box = QGroupBox("Raw")
        raw_layout = QFormLayout(raw_box)
        self.width = self.spin(1, 20000, 3264)
        self.height = self.spin(1, 20000, 2448)
        self.frames = self.spin(1, 10000, 5)
        self.frame_start = self.spin(0, 10000, 0)
        self.header_footer = QCheckBox("header_footer")
        raw_layout.addRow("Size", self.spin_row(("W", self.width), ("H", self.height), ("Frames", self.frames)))
        raw_layout.addRow("Frame start", self.frame_start)
        raw_layout.addRow("Header/Footer", self.header_footer)
        layout.addWidget(raw_box)

        run_box = QGroupBox("Run")
        run_layout = QFormLayout(run_box)
        self.multi_thread = QCheckBox("multi_thread")
        self.thread_count = self.spin(1, 128, 1)
        self.log_enabled = QCheckBox("log_enabled")
        run_layout.addRow("Multi thread", self.multi_thread)
        run_layout.addRow("Thread count", self.thread_count)
        run_layout.addRow("Log", self.log_enabled)
        layout.addWidget(run_box)

        roi_box = QGroupBox("ROI")
        roi_layout = QFormLayout(roi_box)
        self.roi_enabled = QCheckBox("enabled")
        self.roi_up = self.spin(0, 20000, 0)
        self.roi_down = self.spin(0, 20000, 1223)
        self.roi_left = self.spin(0, 20000, 0)
        self.roi_right = self.spin(0, 20000, 1631)
        roi_layout.addRow("Use ROI", self.roi_enabled)
        roi_layout.addRow("Range", self.roi_row(self.roi_up, self.roi_down, self.roi_left, self.roi_right))
        layout.addWidget(roi_box)

        active_box = QGroupBox("Active area")
        active_layout = QFormLayout(active_box)
        self.active_area_enabled = QCheckBox("enabled")
        self.active_up = self.spin(0, 20000, 0)
        self.active_down = self.spin(0, 20000, 1223)
        self.active_left = self.spin(0, 20000, 0)
        self.active_right = self.spin(0, 20000, 1631)
        active_layout.addRow("Use active area", self.active_area_enabled)
        active_layout.addRow("Range", self.roi_row(self.active_up, self.active_down, self.active_left, self.active_right))
        layout.addWidget(active_box)

        layout.addStretch(1)
        return page

    def build_aps_page(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        tests_box = QGroupBox("APS测试项")
        tests_layout = QGridLayout(tests_box)
        for index, item in enumerate(APS_TESTS):
            check = QCheckBox(item)
            check.setChecked(item in {"snoise", "tnoise", "badpixel"})
            self.aps_checks[item] = check
            tests_layout.addWidget(check, index // 3, index % 3)
        layout.addWidget(tests_box)

        threshold_box = QGroupBox("阈值")
        threshold_layout = QFormLayout(threshold_box)
        self.badpixel_threshold = self.double_spin(0.0, 100000.0, 0.19, 6)
        self.badpixel_radius = self.spin(0, 1000, 1)
        self.hotpixel_threshold = self.double_spin(0.0, 100000.0, 120.0, 6)
        threshold_layout.addRow("BadPixel pixel", self.badpixel_threshold)
        threshold_layout.addRow("BadPixel radius", self.badpixel_radius)
        threshold_layout.addRow("HotPixel pixel", self.hotpixel_threshold)
        layout.addWidget(threshold_box)
        layout.addStretch(1)
        return page

    def build_evs_page(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        tests_box = QGroupBox("EVS测试项")
        tests_layout = QGridLayout(tests_box)
        for index, item in enumerate(EVS_TESTS):
            check = QCheckBox(item)
            self.evs_checks[item] = check
            tests_layout.addWidget(check, index // 2, index % 2)
        layout.addWidget(tests_box)

        peak_box = QGroupBox("Peak / Light")
        peak_layout = QFormLayout(peak_box)
        self.peak_num = self.spin(0, 100000, 0)
        self.light_type = QComboBox()
        self.light_type.addItems(["Off", "On", "All"])
        peak_layout.addRow("Peak num", self.peak_num)
        peak_layout.addRow("Light", self.light_type)
        layout.addWidget(peak_box)
        layout.addStretch(1)
        return page

    def build_conditions_page(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        condition_box = QGroupBox("测试条件")
        condition_layout = QFormLayout(condition_box)
        self.repeat_count = self.spin(1, 10000, 1)
        self.exposure_ms = self.double_spin(0.0, 1000000.0, 0.0, 3)
        self.gain = self.double_spin(0.0, 100000.0, 0.0, 3)
        self.temperature = self.double_spin(-273.15, 1000.0, 25.0, 2)
        self.condition_note = QLineEdit()
        condition_layout.addRow("Repeat", self.repeat_count)
        condition_layout.addRow("Exposure ms", self.exposure_ms)
        condition_layout.addRow("Gain", self.gain)
        condition_layout.addRow("Temperature", self.temperature)
        condition_layout.addRow("Note", self.condition_note)
        layout.addWidget(condition_box)
        layout.addStretch(1)
        return page

    def build_settings_page(self) -> QWidget:
        page = QWidget()
        layout = QVBoxLayout(page)

        settings_box = QGroupBox("设置")
        settings_layout = QFormLayout(settings_box)
        self.save_before_run = QCheckBox("run 前保存 profile")
        self.save_before_run.setChecked(True)
        self.auto_load_result = QCheckBox("结束后读取 result.json")
        self.auto_load_result.setChecked(True)
        self.clear_log_on_run = QCheckBox("开始前清空日志")
        self.clear_log_on_run.setChecked(False)
        settings_layout.addRow("Save profile", self.save_before_run)
        settings_layout.addRow("Load result", self.auto_load_result)
        settings_layout.addRow("Clear log", self.clear_log_on_run)
        layout.addWidget(settings_box)
        layout.addStretch(1)
        return page

    def spin(self, minimum: int, maximum: int, value: int) -> QSpinBox:
        widget = QSpinBox()
        widget.setRange(minimum, maximum)
        widget.setValue(value)
        return widget

    def double_spin(self, minimum: float, maximum: float, value: float, decimals: int) -> QDoubleSpinBox:
        widget = QDoubleSpinBox()
        widget.setRange(minimum, maximum)
        widget.setDecimals(decimals)
        widget.setValue(value)
        return widget

    def spin_row(self, *items: tuple[str, QWidget]) -> QWidget:
        row = QWidget()
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 0, 0, 0)
        for label, widget in items:
            layout.addWidget(QLabel(label))
            layout.addWidget(widget)
        layout.addStretch(1)
        return row

    def roi_row(self, up: QSpinBox, down: QSpinBox, left: QSpinBox, right: QSpinBox) -> QWidget:
        return self.spin_row(("Up", up), ("Down", down), ("Left", left), ("Right", right))

    def path_row(self, edit: QLineEdit, slot) -> QWidget:
        row = QWidget()
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 0, 0, 0)
        button = QPushButton("...")
        button.setFixedWidth(36)
        button.clicked.connect(slot)
        layout.addWidget(edit)
        layout.addWidget(button)
        return row

    def show_page(self, index: int) -> None:
        self.stack.setCurrentIndex(index)
        self.tabs.setCurrentIndex(0)
        for i, button in enumerate(self.nav_buttons):
            button.setChecked(i == index)

    def browse_demo(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Select AlgorithmDemo", str(ROOT), "Executable (*.exe)")
        if path:
            self.demo_path.setText(path)

    def browse_profile(self) -> None:
        path, _ = QFileDialog.getSaveFileName(self, "Select profile", self.profile_path.text(), "JSON (*.json)")
        if path:
            self.profile_path.setText(path)

    def browse_raw(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Select raw", str(ROOT), "Raw (*.raw);;All files (*)")
        if path:
            self.raw_path.setText(path)
            self.correct_raw_shape_if_possible()

    def browse_output(self) -> None:
        path = QFileDialog.getExistingDirectory(self, "Select output", self.output_dir.text())
        if path:
            self.output_dir.setText(path)

    def roi_object(self, up: QSpinBox, down: QSpinBox, left: QSpinBox, right: QSpinBox) -> dict:
        return {
            "up": up.value(),
            "down": down.value(),
            "left": left.value(),
            "right": right.value(),
        }

    def selected_tests(self) -> list[dict]:
        if self.mode.currentText() == "EVS":
            return [{"name": name, "enabled": check.isChecked()} for name, check in self.evs_checks.items()]

        tests = []
        for name, check in self.aps_checks.items():
            item: dict = {"name": name, "enabled": check.isChecked()}
            if name == "badpixel":
                item["thresholds"] = {
                    "pixel": self.badpixel_threshold.value(),
                    "radius": self.badpixel_radius.value(),
                }
            elif name == "hotpixel":
                item["thresholds"] = {
                    "pixel": self.hotpixel_threshold.value(),
                }
            tests.append(item)
        return tests

    @staticmethod
    def infer_raw_shape_from_name(path_text: str) -> tuple[int, int, int] | None:
        name = Path(path_text).name.lower()
        match = re.search(r"(\d{2,5})x(\d{2,5}).*?(\d{1,5})\s*frames?", name)
        if not match:
            return None
        return int(match.group(1)), int(match.group(2)), int(match.group(3))

    @staticmethod
    def expected_raw_bytes(width: int, height: int, frames: int, raw_type: str) -> int:
        pixels = width * height
        raw_type = raw_type.lower()
        if raw_type == "raw8":
            one_frame_bytes = pixels
        elif raw_type == "raw10":
            one_frame_bytes = pixels // 4 * 5
        elif raw_type == "raw12":
            one_frame_bytes = pixels // 2 * 3
        else:
            one_frame_bytes = pixels * 2
        return one_frame_bytes * frames

    def correct_raw_shape_if_possible(self, log: bool = True) -> None:
        raw_path = Path(self.raw_path.text())
        if not raw_path.exists():
            return

        actual_bytes = raw_path.stat().st_size
        current_expected = self.expected_raw_bytes(
            self.width.value(),
            self.height.value(),
            self.frames.value(),
            self.raw_type.currentText(),
        )
        if actual_bytes == current_expected:
            return

        inferred = self.infer_raw_shape_from_name(self.raw_path.text())
        if inferred is None:
            if log:
                self.append_log(
                    f"Raw size mismatch before run: raw={actual_bytes}, expected={current_expected}"
                )
            return

        width, height, frames = inferred
        inferred_expected = self.expected_raw_bytes(width, height, frames, self.raw_type.currentText())
        if actual_bytes != inferred_expected:
            if log:
                self.append_log(
                    "Raw size mismatch before run: "
                    f"raw={actual_bytes}, expected={current_expected}, "
                    f"inferred_expected={inferred_expected}"
                )
            return

        old_shape = f"{self.width.value()}x{self.height.value()}, frames={self.frames.value()}"
        self.width.setValue(width)
        self.height.setValue(height)
        self.frames.setValue(frames)
        if log:
            self.append_log(
                f"Raw size auto-corrected from file name: {old_shape} -> {width}x{height}, frames={frames}"
            )

    def build_profile(self) -> dict:
        profile = {
            "version": 1,
            "mode": self.mode.currentText(),
            "sensor": self.sensor.currentText(),
            "multi_thread": self.multi_thread.isChecked(),
            "thread_count": self.thread_count.value(),
            "log_enabled": self.log_enabled.isChecked(),
            "raw": {
                "path": self.raw_path.text(),
                "width": self.width.value(),
                "height": self.height.value(),
                "frames": self.frames.value(),
                "frame_start": self.frame_start.value(),
                "raw_type": self.raw_type.currentText(),
                "pixel_format": self.pixel_format.currentText(),
                "header_footer": self.header_footer.isChecked(),
            },
            "tests": self.selected_tests(),
            "conditions": {
                "repeat_count": self.repeat_count.value(),
                "exposure_ms": self.exposure_ms.value(),
                "gain": self.gain.value(),
                "temperature": self.temperature.value(),
                "note": self.condition_note.text(),
            },
            "output": {
                "directory": self.output_dir.text(),
                "result_json": self.result_json.text() or "result.json",
            },
        }
        if self.roi_enabled.isChecked():
            profile["roi"] = self.roi_object(self.roi_up, self.roi_down, self.roi_left, self.roi_right)
        if self.active_area_enabled.isChecked():
            profile["active_area"] = self.roi_object(self.active_up, self.active_down, self.active_left, self.active_right)
        return profile

    def save_profile(self) -> None:
        self.correct_raw_shape_if_possible()
        profile_path = Path(self.profile_path.text())
        profile_path.parent.mkdir(parents=True, exist_ok=True)
        profile_path.write_text(json.dumps(self.build_profile(), indent=2), encoding="utf-8")
        self.save_last_session()
        self.append_log(f"Saved profile: {profile_path}")

    def save_last_session(self) -> None:
        LAST_SESSION.parent.mkdir(parents=True, exist_ok=True)
        session = {
            "demo_path": self.demo_path.text(),
            "profile_path": self.profile_path.text(),
            "raw_path": self.raw_path.text(),
            "output_dir": self.output_dir.text(),
            "result_json": self.result_json.text(),
            "mode": self.mode.currentText(),
            "sensor": self.sensor.currentText(),
            "raw_type": self.raw_type.currentText(),
            "pixel_format": self.pixel_format.currentText(),
            "width": self.width.value(),
            "height": self.height.value(),
            "frames": self.frames.value(),
            "frame_start": self.frame_start.value(),
            "header_footer": self.header_footer.isChecked(),
            "multi_thread": self.multi_thread.isChecked(),
            "thread_count": self.thread_count.value(),
            "log_enabled": self.log_enabled.isChecked(),
            "roi_enabled": self.roi_enabled.isChecked(),
            "roi": self.roi_object(self.roi_up, self.roi_down, self.roi_left, self.roi_right),
            "active_area_enabled": self.active_area_enabled.isChecked(),
            "active_area": self.roi_object(self.active_up, self.active_down, self.active_left, self.active_right),
            "aps_tests": {name: check.isChecked() for name, check in self.aps_checks.items()},
            "evs_tests": {name: check.isChecked() for name, check in self.evs_checks.items()},
            "badpixel_threshold": self.badpixel_threshold.value(),
            "badpixel_radius": self.badpixel_radius.value(),
            "hotpixel_threshold": self.hotpixel_threshold.value(),
            "peak_num": self.peak_num.value(),
            "light_type": self.light_type.currentText(),
            "repeat_count": self.repeat_count.value(),
            "exposure_ms": self.exposure_ms.value(),
            "gain": self.gain.value(),
            "temperature": self.temperature.value(),
            "condition_note": self.condition_note.text(),
            "save_before_run": self.save_before_run.isChecked(),
            "auto_load_result": self.auto_load_result.isChecked(),
            "clear_log_on_run": self.clear_log_on_run.isChecked(),
        }
        LAST_SESSION.write_text(json.dumps(session, indent=2), encoding="utf-8")

    def load_last_session(self) -> None:
        if not LAST_SESSION.exists():
            return
        try:
            session = json.loads(LAST_SESSION.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            return

        self.demo_path.setText(session.get("demo_path", self.demo_path.text()))
        self.profile_path.setText(session.get("profile_path", self.profile_path.text()))
        self.raw_path.setText(session.get("raw_path", self.raw_path.text()))
        self.output_dir.setText(session.get("output_dir", self.output_dir.text()))
        self.result_json.setText(session.get("result_json", self.result_json.text()))
        self.set_combo(self.mode, session.get("mode"))
        self.set_combo(self.sensor, session.get("sensor"))
        self.set_combo(self.raw_type, session.get("raw_type"))
        self.set_combo(self.pixel_format, session.get("pixel_format"))
        self.width.setValue(int(session.get("width", self.width.value())))
        self.height.setValue(int(session.get("height", self.height.value())))
        self.frames.setValue(int(session.get("frames", self.frames.value())))
        self.frame_start.setValue(int(session.get("frame_start", self.frame_start.value())))
        self.header_footer.setChecked(bool(session.get("header_footer", self.header_footer.isChecked())))
        self.multi_thread.setChecked(bool(session.get("multi_thread", self.multi_thread.isChecked())))
        self.thread_count.setValue(int(session.get("thread_count", self.thread_count.value())))
        self.log_enabled.setChecked(bool(session.get("log_enabled", self.log_enabled.isChecked())))
        self.roi_enabled.setChecked(bool(session.get("roi_enabled", self.roi_enabled.isChecked())))
        self.load_roi(session.get("roi", {}), self.roi_up, self.roi_down, self.roi_left, self.roi_right)
        self.active_area_enabled.setChecked(bool(session.get("active_area_enabled", self.active_area_enabled.isChecked())))
        self.load_roi(session.get("active_area", {}), self.active_up, self.active_down, self.active_left, self.active_right)
        for name, checked in session.get("aps_tests", {}).items():
            if name in self.aps_checks:
                self.aps_checks[name].setChecked(bool(checked))
        for name, checked in session.get("evs_tests", {}).items():
            if name in self.evs_checks:
                self.evs_checks[name].setChecked(bool(checked))
        self.badpixel_threshold.setValue(float(session.get("badpixel_threshold", self.badpixel_threshold.value())))
        self.badpixel_radius.setValue(int(session.get("badpixel_radius", self.badpixel_radius.value())))
        self.hotpixel_threshold.setValue(float(session.get("hotpixel_threshold", self.hotpixel_threshold.value())))
        self.peak_num.setValue(int(session.get("peak_num", self.peak_num.value())))
        self.set_combo(self.light_type, session.get("light_type"))
        self.repeat_count.setValue(int(session.get("repeat_count", self.repeat_count.value())))
        self.exposure_ms.setValue(float(session.get("exposure_ms", self.exposure_ms.value())))
        self.gain.setValue(float(session.get("gain", self.gain.value())))
        self.temperature.setValue(float(session.get("temperature", self.temperature.value())))
        self.condition_note.setText(session.get("condition_note", self.condition_note.text()))
        self.save_before_run.setChecked(bool(session.get("save_before_run", self.save_before_run.isChecked())))
        self.auto_load_result.setChecked(bool(session.get("auto_load_result", self.auto_load_result.isChecked())))
        self.clear_log_on_run.setChecked(bool(session.get("clear_log_on_run", self.clear_log_on_run.isChecked())))
        self.correct_raw_shape_if_possible(log=False)

    def load_roi(self, data: dict, up: QSpinBox, down: QSpinBox, left: QSpinBox, right: QSpinBox) -> None:
        up.setValue(int(data.get("up", up.value())))
        down.setValue(int(data.get("down", down.value())))
        left.setValue(int(data.get("left", left.value())))
        right.setValue(int(data.get("right", right.value())))

    def set_combo(self, combo: QComboBox, value: str | None) -> None:
        if value is None:
            return
        index = combo.findText(value)
        if index >= 0:
            combo.setCurrentIndex(index)

    def toggle_run(self) -> None:
        if self.running:
            self.stop_demo()
        else:
            self.run_demo()

    def run_demo(self) -> None:
        if self.mode.currentText() != "APS":
            QMessageBox.information(
                self,
                "暂不支持",
                "当前 AlgorithmDemo 的 profile 执行器只支持 APS，EVS 页面暂时只作为配置占位。",
            )
            self.statusBar().showMessage("Idle: EVS is not supported yet")
            return

        reply = QMessageBox.question(
            self,
            "开始运行",
            "是否保存当前 profile 并开始运行？",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if reply != QMessageBox.StandardButton.Yes:
            return

        if self.clear_log_on_run.isChecked():
            self.log_output.clear()
        if self.save_before_run.isChecked():
            self.save_profile()
        else:
            self.save_last_session()

        demo = Path(self.demo_path.text())
        profile = Path(self.profile_path.text())
        if not demo.exists():
            QMessageBox.warning(self, "Missing executable", f"Demo executable does not exist:\n{demo}")
            self.statusBar().showMessage("Failed: missing executable")
            return

        try:
            self.backup_existing_result()
        except OSError as exc:
            QMessageBox.warning(self, "Backup failed", f"Failed to backup old result.json:\n{exc}")
            self.statusBar().showMessage("Failed: result backup error")
            return

        self.append_log(f"Running: {demo} --profile {profile}")
        self.process = QProcess(self)
        self.process.setProgram(str(demo))
        self.process.setArguments(["--profile", str(profile)])
        self.process.readyReadStandardOutput.connect(self.read_stdout)
        self.process.readyReadStandardError.connect(self.read_stderr)
        self.process.finished.connect(self.process_finished)
        self.process.start()
        self.running = True
        self.stop_requested = False
        self.run_button.setText("停止")
        self.statusBar().showMessage(f"Running: {profile}")

    def stop_demo(self) -> None:
        if self.process is None:
            return
        reply = QMessageBox.question(
            self,
            "停止运行",
            "是否停止当前运行进程？",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            QMessageBox.StandardButton.No,
        )
        if reply != QMessageBox.StandardButton.Yes:
            return
        self.stop_requested = True
        self.append_log("Stopping...")
        self.statusBar().showMessage("Stopping")
        self.process.terminate()
        QTimer.singleShot(3000, self.kill_if_running)

    def kill_if_running(self) -> None:
        if self.process is not None and self.process.state() != QProcess.NotRunning:
            self.process.kill()

    def read_stdout(self) -> None:
        if self.process is not None:
            text = bytes(self.process.readAllStandardOutput()).decode(errors="replace")
            self.append_log(text.rstrip())

    def read_stderr(self) -> None:
        if self.process is not None:
            text = bytes(self.process.readAllStandardError()).decode(errors="replace")
            self.append_log(text.rstrip())

    def process_finished(self, exit_code: int = 0, exit_status=None) -> None:
        self.running = False
        self.run_button.setText("开始")
        self.append_log(f"Run finished. exit={exit_code}")
        if self.stop_requested:
            self.statusBar().showMessage("Stopped")
            self.stop_requested = False
            return
        if exit_code != 0:
            self.statusBar().showMessage(f"Failed: exit={exit_code}")
        else:
            self.statusBar().showMessage(f"Finished: exit={exit_code}")
        if self.auto_load_result.isChecked():
            self.load_result()

    def append_log(self, text: str) -> None:
        if text:
            self.log_output.appendPlainText(text)

    def current_result_path(self) -> Path:
        return Path(self.output_dir.text()) / (self.result_json.text() or "result.json")

    def backup_existing_result(self) -> None:
        result_path = self.current_result_path()
        if not result_path.exists():
            return
        stat = result_path.stat()
        created_at = datetime.fromtimestamp(stat.st_ctime)
        stamp = created_at.strftime("%Y%m%d_%H%M%S")
        backup_path = self.unique_backup_path(result_path, stamp)
        result_path.rename(backup_path)
        self.append_log(f"Backed up old result: {backup_path}")

    def unique_backup_path(self, result_path: Path, stamp: str) -> Path:
        base = result_path.with_name(f"{result_path.stem}_{stamp}{result_path.suffix}")
        if not base.exists():
            return base
        for index in range(1, 1000):
            candidate = result_path.with_name(f"{result_path.stem}_{stamp}_{index}{result_path.suffix}")
            if not candidate.exists():
                return candidate
        raise OSError(f"too many backup files for {result_path}")

    def load_result(self) -> None:
        result_path = self.current_result_path()
        if not result_path.exists():
            self.append_log(f"Result does not exist: {result_path}")
            return
        try:
            result = json.loads(result_path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            self.append_log(f"Failed to parse result: {exc}")
            return
        self.append_log(f"Status: {result.get('status')}")
        for item in result.get("tests", []):
            self.append_log(f"{item.get('name')}: {item.get('status')} {item.get('ms')} ms")


def main() -> int:
    app = QApplication(sys.argv)
    window = AlgorithmTestWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())

from __future__ import annotations

import json
import sys
from pathlib import Path

try:
    from PySide6.QtCore import QProcess
    from PySide6.QtWidgets import (
        QApplication,
        QCheckBox,
        QComboBox,
        QFileDialog,
        QFormLayout,
        QGridLayout,
        QGroupBox,
        QHBoxLayout,
        QLabel,
        QLineEdit,
        QMainWindow,
        QMessageBox,
        QPushButton,
        QPlainTextEdit,
        QSpinBox,
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


class AlgorithmTestWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("Algorithm Debug Test")
        self.process: QProcess | None = None
        self.checks: dict[str, QCheckBox] = {}

        central = QWidget()
        self.setCentralWidget(central)
        root = QVBoxLayout(central)

        config_box = QGroupBox("Run")
        config_layout = QFormLayout(config_box)

        self.demo_path = QLineEdit(str(DEFAULT_DEMO))
        config_layout.addRow("Demo exe", self._path_row(self.demo_path, self.browse_demo))

        self.profile_path = QLineEdit(str(DEFAULT_PROFILE))
        config_layout.addRow("Profile", self._path_row(self.profile_path, self.browse_profile))

        self.raw_path = QLineEdit(str(DEFAULT_RAW))
        config_layout.addRow("Raw", self._path_row(self.raw_path, self.browse_raw))

        self.output_dir = QLineEdit(str(DEFAULT_OUTPUT))
        config_layout.addRow("Output", self._path_row(self.output_dir, self.browse_output))

        self.sensor = QComboBox()
        self.sensor.addItems(["ALP_003CA", "ALP_003AA", "ALP_003BA", "ALP_003BB", "ALP_004AB", "ALP_014AA", "ALP_014BA"])
        config_layout.addRow("Sensor", self.sensor)

        self.raw_type = QComboBox()
        self.raw_type.addItems(["UNPACK10", "UNPACK12", "RAW8", "RAW10", "RAW12"])
        config_layout.addRow("Raw type", self.raw_type)

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
        config_layout.addRow("Pixel format", self.pixel_format)

        size_row = QWidget()
        size_layout = QHBoxLayout(size_row)
        size_layout.setContentsMargins(0, 0, 0, 0)
        self.width = QSpinBox()
        self.width.setRange(1, 20000)
        self.width.setValue(3264)
        self.height = QSpinBox()
        self.height.setRange(1, 20000)
        self.height.setValue(2448)
        self.frames = QSpinBox()
        self.frames.setRange(1, 10000)
        self.frames.setValue(5)
        size_layout.addWidget(QLabel("W"))
        size_layout.addWidget(self.width)
        size_layout.addWidget(QLabel("H"))
        size_layout.addWidget(self.height)
        size_layout.addWidget(QLabel("Frames"))
        size_layout.addWidget(self.frames)
        config_layout.addRow("Raw size", size_row)

        self.thread_count = QSpinBox()
        self.thread_count.setRange(1, 128)
        self.thread_count.setValue(1)
        config_layout.addRow("Threads", self.thread_count)
        root.addWidget(config_box)

        tests_box = QGroupBox("APS Tests")
        tests_layout = QGridLayout(tests_box)
        for index, item in enumerate(["snoise", "tnoise", "badpixel", "hotpixel", "datamean", "dsnu"]):
            check = QCheckBox(item)
            check.setChecked(item in {"snoise", "tnoise", "badpixel"})
            self.checks[item] = check
            tests_layout.addWidget(check, index // 3, index % 3)
        root.addWidget(tests_box)

        button_row = QWidget()
        button_layout = QHBoxLayout(button_row)
        button_layout.setContentsMargins(0, 0, 0, 0)
        save_button = QPushButton("Save profile")
        save_button.clicked.connect(self.save_profile)
        run_button = QPushButton("Run")
        run_button.clicked.connect(self.run_demo)
        load_button = QPushButton("Load result")
        load_button.clicked.connect(self.load_result)
        button_layout.addWidget(save_button)
        button_layout.addWidget(run_button)
        button_layout.addWidget(load_button)
        root.addWidget(button_row)

        self.output = QPlainTextEdit()
        self.output.setReadOnly(True)
        root.addWidget(self.output, 1)

        self.load_last_session()
        self.resize(900, 720)

    def _path_row(self, edit: QLineEdit, slot) -> QWidget:
        row = QWidget()
        layout = QHBoxLayout(row)
        layout.setContentsMargins(0, 0, 0, 0)
        button = QPushButton("...")
        button.setFixedWidth(36)
        button.clicked.connect(slot)
        layout.addWidget(edit)
        layout.addWidget(button)
        return row

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

    def browse_output(self) -> None:
        path = QFileDialog.getExistingDirectory(self, "Select output", self.output_dir.text())
        if path:
            self.output_dir.setText(path)

    def build_profile(self) -> dict:
        tests = []
        for name, check in self.checks.items():
            tests.append({"name": name, "enabled": check.isChecked()})
        return {
            "version": 1,
            "mode": "APS",
            "sensor": self.sensor.currentText(),
            "thread_count": self.thread_count.value(),
            "log_enabled": False,
            "raw": {
                "path": self.raw_path.text(),
                "width": self.width.value(),
                "height": self.height.value(),
                "frames": self.frames.value(),
                "raw_type": self.raw_type.currentText(),
                "pixel_format": self.pixel_format.currentText(),
                "header_footer": False,
            },
            "tests": tests,
            "output": {
                "directory": self.output_dir.text(),
                "result_json": "result.json",
            },
        }

    def save_profile(self) -> None:
        profile_path = Path(self.profile_path.text())
        profile_path.parent.mkdir(parents=True, exist_ok=True)
        profile = self.build_profile()
        profile_path.write_text(json.dumps(profile, indent=2), encoding="utf-8")
        self.save_last_session()
        self.output.appendPlainText(f"Saved profile: {profile_path}")

    def save_last_session(self) -> None:
        LAST_SESSION.parent.mkdir(parents=True, exist_ok=True)
        session = {
            "demo_path": self.demo_path.text(),
            "profile_path": self.profile_path.text(),
            "raw_path": self.raw_path.text(),
            "output_dir": self.output_dir.text(),
            "sensor": self.sensor.currentText(),
            "raw_type": self.raw_type.currentText(),
            "pixel_format": self.pixel_format.currentText(),
            "width": self.width.value(),
            "height": self.height.value(),
            "frames": self.frames.value(),
            "thread_count": self.thread_count.value(),
            "tests": {name: check.isChecked() for name, check in self.checks.items()},
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
        self.sensor.setCurrentText(session.get("sensor", self.sensor.currentText()))
        self.raw_type.setCurrentText(session.get("raw_type", self.raw_type.currentText()))
        self.pixel_format.setCurrentText(session.get("pixel_format", self.pixel_format.currentText()))
        self.width.setValue(int(session.get("width", self.width.value())))
        self.height.setValue(int(session.get("height", self.height.value())))
        self.frames.setValue(int(session.get("frames", self.frames.value())))
        self.thread_count.setValue(int(session.get("thread_count", self.thread_count.value())))
        tests = session.get("tests", {})
        for name, checked in tests.items():
            if name in self.checks:
                self.checks[name].setChecked(bool(checked))

    def run_demo(self) -> None:
        self.save_profile()
        demo = Path(self.demo_path.text())
        profile = Path(self.profile_path.text())
        if not demo.exists():
            QMessageBox.warning(self, "Missing executable", f"Demo executable does not exist:\n{demo}")
            return
        self.output.appendPlainText(f"Running: {demo} --profile {profile}")
        self.process = QProcess(self)
        self.process.setProgram(str(demo))
        self.process.setArguments(["--profile", str(profile)])
        self.process.readyReadStandardOutput.connect(self.read_stdout)
        self.process.readyReadStandardError.connect(self.read_stderr)
        self.process.finished.connect(self.process_finished)
        self.process.start()

    def read_stdout(self) -> None:
        if self.process is not None:
            text = bytes(self.process.readAllStandardOutput()).decode(errors="replace")
            self.output.appendPlainText(text.rstrip())

    def read_stderr(self) -> None:
        if self.process is not None:
            text = bytes(self.process.readAllStandardError()).decode(errors="replace")
            self.output.appendPlainText(text.rstrip())

    def process_finished(self) -> None:
        self.output.appendPlainText("Run finished.")
        self.load_result()

    def load_result(self) -> None:
        result_path = Path(self.output_dir.text()) / "result.json"
        if not result_path.exists():
            self.output.appendPlainText(f"Result does not exist: {result_path}")
            return
        try:
            result = json.loads(result_path.read_text(encoding="utf-8"))
        except json.JSONDecodeError as exc:
            self.output.appendPlainText(f"Failed to parse result: {exc}")
            return
        self.output.appendPlainText(f"Status: {result.get('status')}")
        for item in result.get("tests", []):
            self.output.appendPlainText(
                f"{item.get('name')}: {item.get('status')} {item.get('ms')} ms"
            )


def main() -> int:
    app = QApplication(sys.argv)
    window = AlgorithmTestWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())

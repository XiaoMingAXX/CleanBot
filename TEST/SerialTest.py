import sys
import struct
import time
import threading
from collections import deque, defaultdict

import serial
import serial.tools.list_ports

from PySide6.QtCore import Qt, QTimer, Signal, QObject
from PySide6.QtWidgets import (
    QApplication, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QLabel, QComboBox, QDoubleSpinBox, QSpinBox, QTextEdit, QGridLayout,
    QGroupBox, QMessageBox, QCheckBox
)

# ----------------- 协议常量 -----------------
HEADER = b'\x55\xAA'
VERSION = 0x01
MSG_CONTROL_CMD = 0x10
MSG_IMU = 0x20
MSG_WHEEL = 0x21
MSG_SENSOR = 0x22
MSG_ACK = 0x24

WORK_MODES = [
    ("Idle", 0),
    ("Auto", 1),
    ("Edge", 2),
    ("Bow", 3),
    ("Remote", 4),
    ("Dock", 5),
]

FAN_LEVEL_MAX = 5
PUMP_LEVEL_MAX = 5
BRUSH_LEVEL_MAX = 3  # 0~3 -> OFF/LOW/HIGH(复用)
TARGET_FREQ = {
    MSG_WHEEL: 200,
    MSG_IMU: 100,
    MSG_SENSOR: 50,
}
MAX_PAYLOAD_LEN = 128

# CRC16-CCITT
def crc16_ccitt(data, init=0xFFFF):
    crc = init
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if crc & 0x8000:
                crc = ((crc << 1) ^ 0x1021) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    return crc

# ----------------- GUI 与通讯桥 -----------------
class SerialWorker(QObject):
    telemetry_received = Signal(int, dict)   # msg_id, payload dict
    log_message = Signal(str)
    ack_received = Signal(float, dict)       # rtt_ms, payload dict
    connection_state = Signal(bool)

    def __init__(self, port, baud=921600):
        super().__init__()
        self.port = port
        self.baud = baud
        self.ser = None
        self.running = False
        self.rx_buffer = bytearray()
        self._reset_parser()
        self.last_seq = defaultdict(lambda: None)
        self.ack_wait = {}
        self.lock = threading.Lock()

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baud, timeout=0.01)
            self.running = True
            threading.Thread(target=self.rx_loop, daemon=True).start()
            self.connection_state.emit(True)
            self.log_message.emit(f"[INFO] Serial connected on {self.port}")
        except serial.SerialException as e:
            self.log_message.emit(f"[ERROR] Serial open failed: {e}")
            self.connection_state.emit(False)

    def stop(self):
        self.running = False
        if self.ser and self.ser.is_open:
            self.ser.close()
            self.connection_state.emit(False)
            self.log_message.emit("[INFO] Serial closed")

    def send_control(self, payload, seq):
        frame = bytearray()
        frame += HEADER
        frame.append(VERSION)
        frame += struct.pack('<H', len(payload))
        frame.append(MSG_CONTROL_CMD)
        frame.append(seq)
        frame += payload
        crc = crc16_ccitt(frame[2:])
        frame += struct.pack('<H', crc)

        with self.lock:
            try:
                if self.ser and self.ser.is_open:
                    self.ser.write(frame)
                    self.ack_wait[seq] = time.time()
            except serial.SerialException as e:
                self.log_message.emit(f"[ERROR] Serial send failed: {e}")
                self.connection_state.emit(False)

    def rx_loop(self):
        while self.running:
            try:
                chunk = self.ser.read(256)
            except Exception as e:
                self.log_message.emit(f"[ERROR] Serial read failed: {e}")
                self.connection_state.emit(False)
                break
            if chunk:
                self.rx_buffer.extend(chunk)
                self.process_buffer()
        self.connection_state.emit(False)

    def _reset_parser(self):
        self.parser_state = {
            "state": "WAIT_HEADER0",
            "version": 0,
            "length": 0,
            "msg_id": 0,
            "seq": 0,
            "payload": bytearray(),
            "crc": 0,
        }

    def process_buffer(self):
        if not self.rx_buffer:
            return
        data = bytes(self.rx_buffer)
        self.rx_buffer.clear()
        for byte in data:
            self._consume_byte(byte)

    def _consume_byte(self, byte):
        state = self.parser_state
        current = state["state"]

        if current == "WAIT_HEADER0":
            if byte == HEADER[0]:
                state["state"] = "WAIT_HEADER1"
        elif current == "WAIT_HEADER1":
            if byte == HEADER[1]:
                state["state"] = "READ_VERSION"
            else:
                state["state"] = "WAIT_HEADER0"
        elif current == "READ_VERSION":
            state["version"] = byte
            state["state"] = "READ_LEN_L"
        elif current == "READ_LEN_L":
            state["length"] = byte
            state["state"] = "READ_LEN_H"
        elif current == "READ_LEN_H":
            state["length"] |= (byte << 8)
            if state["length"] > MAX_PAYLOAD_LEN:
                self.log_message.emit("[WARN] Payload length overflow, reset parser")
                self._reset_parser()
            else:
                state["payload"] = bytearray()
                state["state"] = "READ_MSG_ID"
        elif current == "READ_MSG_ID":
            state["msg_id"] = byte
            state["state"] = "READ_SEQ"
        elif current == "READ_SEQ":
            state["seq"] = byte
            if state["length"] == 0:
                state["state"] = "READ_CRC_L"
            else:
                state["state"] = "READ_PAYLOAD"
        elif current == "READ_PAYLOAD":
            state["payload"].append(byte)
            if len(state["payload"]) >= state["length"]:
                state["state"] = "READ_CRC_L"
        elif current == "READ_CRC_L":
            state["crc"] = byte
            state["state"] = "READ_CRC_H"
        elif current == "READ_CRC_H":
            state["crc"] |= (byte << 8)
            frame_core = bytearray()
            frame_core.append(state["version"])
            frame_core += struct.pack('<H', state["length"])
            frame_core.append(state["msg_id"])
            frame_core.append(state["seq"])
            frame_core += state["payload"]
            calc = crc16_ccitt(frame_core)
            if calc != state["crc"]:
                self.log_message.emit("[WARN] CRC mismatch, drop frame")
            elif state["version"] != VERSION:
                self.log_message.emit("[WARN] Version mismatch, drop frame")
            else:
                payload = bytes(state["payload"])
                msg_id = state["msg_id"]
                seq = state["seq"]
                data = self.parse_payload(msg_id, payload)
                if data is None:
                    data = {}
                self.telemetry_received.emit(msg_id, data)
                if msg_id == MSG_ACK and seq in self.ack_wait:
                    rtt = (time.time() - self.ack_wait.pop(seq)) * 1000.0
                    self.ack_received.emit(rtt, data)
            self._reset_parser()
        else:
            self._reset_parser()

    def parse_payload(self, msg_id, payload):
        try:
            if msg_id == MSG_WHEEL and len(payload) == 16:
                vals = struct.unpack('<ffff', payload)
                return {"left_angle_deg": vals[0], "left_speed_mps": vals[1],
                        "right_angle_deg": vals[2], "right_speed_mps": vals[3]}
            if msg_id == MSG_IMU and len(payload) == 36:
                vals = struct.unpack('<fffffffff', payload)
                return {
                    "ax": vals[0], "ay": vals[1], "az": vals[2],
                    "gx": vals[3], "gy": vals[4], "gz": vals[5],
                    "roll": vals[6], "pitch": vals[7], "yaw": vals[8]
                }
            if msg_id == MSG_SENSOR and len(payload) == 9:
                return {
                    "bumper_left": payload[0],
                    "bumper_right": payload[1],
                    "ir_down0": payload[2],
                    "ir_down1": payload[3],
                    "ir_down2": payload[4],
                    "fault": payload[5],
                    "heartbeat": payload[6],
                    "dock_status": payload[7],
                    "reserved": payload[8]
                }
            if msg_id == MSG_ACK and len(payload) >= 2:
                return {"cmd_id": payload[0], "status": payload[1],
                        "info": payload[2] if len(payload) > 2 else 0}
        except struct.error:
            self.log_message.emit(f"[WARN] Payload decode error (msg {msg_id})")
        return {}

# ----------------- 主界面 -----------------
class ControlPanel(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("CleanBot USB 调试工具")
        self.resize(1100, 700)

        self.serial = None
        self.seq_counter = 0
        self.freq_stats = {msg: deque(maxlen=2000) for msg in TARGET_FREQ}
        self.last_recv_time = {}

        main_layout = QVBoxLayout()
        top_layout = QHBoxLayout()
        main_layout.addLayout(top_layout)

        # 左侧：控制区
        ctrl_box = QGroupBox("控制")
        top_layout.addWidget(ctrl_box)
        ctrl_layout = QGridLayout()
        ctrl_box.setLayout(ctrl_layout)

        row = 0
        ctrl_layout.addWidget(QLabel("串口："), row, 0)
        self.port_combo = QComboBox()
        for port in serial.tools.list_ports.comports():
            self.port_combo.addItem(port.device)
        ctrl_layout.addWidget(self.port_combo, row, 1)
        self.connect_btn = QPushButton("连接")
        ctrl_layout.addWidget(self.connect_btn, row, 2)

        row += 1
        ctrl_layout.addWidget(QLabel("模式："), row, 0)
        self.mode_combo = QComboBox()
        for name, val in WORK_MODES:
            self.mode_combo.addItem(f"{name} ({val})", val)
        ctrl_layout.addWidget(self.mode_combo, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("左轮 (m/s)："), row, 0)
        self.left_speed = QDoubleSpinBox()
        self.left_speed.setRange(-1.5, 1.5)
        self.left_speed.setSingleStep(0.05)
        ctrl_layout.addWidget(self.left_speed, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("右轮 (m/s)："), row, 0)
        self.right_speed = QDoubleSpinBox()
        self.right_speed.setRange(-1.5, 1.5)
        self.right_speed.setSingleStep(0.05)
        ctrl_layout.addWidget(self.right_speed, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("风机档 (0-5)："), row, 0)
        self.fan_level = QSpinBox()
        self.fan_level.setRange(0, FAN_LEVEL_MAX)
        ctrl_layout.addWidget(self.fan_level, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("水箱档 (0-5)："), row, 0)
        self.pump_level = QSpinBox()
        self.pump_level.setRange(0, PUMP_LEVEL_MAX)
        ctrl_layout.addWidget(self.pump_level, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("左边刷 (0-3)："), row, 0)
        self.brush_left = QSpinBox()
        self.brush_left.setRange(0, BRUSH_LEVEL_MAX)
        ctrl_layout.addWidget(self.brush_left, row, 1)

        row += 1
        ctrl_layout.addWidget(QLabel("右边刷 (0-3)："), row, 0)
        self.brush_right = QSpinBox()
        ctrl_layout.addWidget(self.brush_right, row, 1)

        row += 1
        self.need_ack = QCheckBox("need_ack")
        ctrl_layout.addWidget(self.need_ack, row, 0)
        self.send_btn = QPushButton("发送 CONTROL_CMD")
        ctrl_layout.addWidget(self.send_btn, row, 1)

        # 中间：传感器显示
        sensor_box = QGroupBox("传感器/状态")
        top_layout.addWidget(sensor_box)
        sensor_layout = QGridLayout()
        sensor_box.setLayout(sensor_layout)

        self.labels = {}
        sensors = [
            ("left_angle_deg", "左轮角度 (°)"),
            ("left_speed_mps", "左轮速度 (m/s)"),
            ("right_angle_deg", "右轮角度 (°)"),
            ("right_speed_mps", "右轮速度 (m/s)"),
            ("ax", "Ax (m/s²)"), ("ay", "Ay"), ("az", "Az"),
            ("gx", "Gx (rad/s)"), ("gy", "Gy"), ("gz", "Gz"),
            ("roll", "Roll (rad)"), ("pitch", "Pitch"), ("yaw", "Yaw"),
            ("bumper_left", "左碰撞"), ("bumper_right", "右碰撞"),
            ("ir_down0", "下视0"), ("ir_down1", "下视1"), ("ir_down2", "下视2"),
            ("heartbeat", "心跳"), ("dock_status", "Dock状态"), ("fault", "故障掩码"),
        ]
        for row, (key, name) in enumerate(sensors):
            sensor_layout.addWidget(QLabel(name), row, 0)
            lbl = QLabel("--")
            sensor_layout.addWidget(lbl, row, 1)
            self.labels[key] = lbl

        # 右侧：通讯统计
        stat_box = QGroupBox("通讯监控")
        top_layout.addWidget(stat_box)
        stat_layout = QGridLayout()
        stat_box.setLayout(stat_layout)

        self.freq_labels = {}
        for i, (msg, freq) in enumerate(TARGET_FREQ.items()):
            lbl = QLabel(f"{freq} Hz")
            stat_layout.addWidget(QLabel(f"MSG 0x{msg:02X} 频率"), i, 0)
            stat_layout.addWidget(lbl, i, 1)
            self.freq_labels[msg] = lbl

        self.ack_label = QLabel("--")
        stat_layout.addWidget(QLabel("ACK RTT (ms)"), len(TARGET_FREQ), 0)
        stat_layout.addWidget(self.ack_label, len(TARGET_FREQ), 1)

        self.log_area = QTextEdit()
        self.log_area.setReadOnly(True)
        main_layout.addWidget(self.log_area)

        self.setLayout(main_layout)

        # 事件绑定
        self.connect_btn.clicked.connect(self.handle_connect)
        self.send_btn.clicked.connect(self.send_control)
        self.mode_combo.currentIndexChanged.connect(self.on_mode_change)

        # 周期刷新频率
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_stats)
        self.timer.start(500)

        # 键盘焦点
        self.setFocusPolicy(Qt.StrongFocus)

    def keyPressEvent(self, event):
        key = event.key()
        if key == Qt.Key_1:
            self.mode_combo.setCurrentIndex(0)
        elif key == Qt.Key_2:
            self.mode_combo.setCurrentIndex(1)
        elif key == Qt.Key_3:
            self.mode_combo.setCurrentIndex(2)
        elif key == Qt.Key_4:
            self.mode_combo.setCurrentIndex(3)
        elif key == Qt.Key_5:
            self.mode_combo.setCurrentIndex(4)
        elif key == Qt.Key_6:
            self.mode_combo.setCurrentIndex(5)
        elif key == Qt.Key_Q:
            self.left_speed.setValue(self.left_speed.value() + 0.1)
        elif key == Qt.Key_W:
            self.right_speed.setValue(self.right_speed.value() + 0.1)
        elif key == Qt.Key_A:
            self.fan_level.setValue(min(self.fan_level.value() + 1, FAN_LEVEL_MAX))
        elif key == Qt.Key_S:
            self.fan_level.setValue(max(self.fan_level.value() - 1, 0))
        elif key == Qt.Key_Z:
            self.pump_level.setValue(min(self.pump_level.value() + 1, PUMP_LEVEL_MAX))
        elif key == Qt.Key_X:
            self.pump_level.setValue(max(self.pump_level.value() - 1, 0))
        elif key == Qt.Key_E:
            self.brush_left.setValue((self.brush_left.value() + 1) % (BRUSH_LEVEL_MAX + 1))
        elif key == Qt.Key_R:
            self.brush_right.setValue((self.brush_right.value() + 1) % (BRUSH_LEVEL_MAX + 1))
        elif key == Qt.Key_Space:
            self.send_control()
        else:
            super().keyPressEvent(event)

    def handle_connect(self):
        if self.serial:
            self.serial.stop()
            self.serial = None
            self.connect_btn.setText("连接")
            return

        port = self.port_combo.currentText()
        if not port:
            QMessageBox.warning(self, "提示", "没有可用串口")
            return

        self.serial = SerialWorker(port)
        self.serial.telemetry_received.connect(self.handle_telemetry)
        self.serial.log_message.connect(self.log_area.append)
        self.serial.ack_received.connect(self.handle_ack)
        self.serial.connection_state.connect(self.on_connection_state)
        self.serial.start()
        self.connect_btn.setText("断开")

    def on_connection_state(self, connected):
        if not connected and self.serial:
            self.log_area.append("[WARN] 连接中断")
            self.connect_btn.setText("连接")
            self.serial = None

    def on_mode_change(self):
        if self.mode_combo.currentData() == 5:  # Dock
            self.log_area.append("[INFO] 已切换 Dock 模式，等待对接反馈")

    def build_control_payload(self):
        left = self.left_speed.value()
        right = self.right_speed.value()
        work_mode = self.mode_combo.currentData()
        fan = self.fan_level.value()
        pump = self.pump_level.value()
        brush_l = self.brush_left.value()
        brush_r = self.brush_right.value()
        need_ack = 1 if self.need_ack.isChecked() else 0
        reserved = 0

        payload = struct.pack(
            '<ffBBBBBBB',
            left, right,
            work_mode, brush_l, brush_r, fan, pump,
            need_ack, reserved
        )
        return payload

    def send_control(self):
        if not self.serial:
            QMessageBox.warning(self, "提示", "尚未连接串口")
            return
        payload = self.build_control_payload()
        seq = self.seq_counter & 0xFF
        self.seq_counter += 1
        self.serial.send_control(payload, seq)
        self.log_area.append(f"[TX] CONTROL_CMD seq={seq}")

    def handle_telemetry(self, msg_id, data):
        now = time.time()
        if msg_id in self.freq_stats:
            self.freq_stats[msg_id].append(now)
        self.last_recv_time[msg_id] = now

        # 更新显示
        for key, value in data.items():
            if key in self.labels:
                self.labels[key].setText(str(value))

        # 特殊提示
        if msg_id == MSG_SENSOR:
            dock = data.get("dock_status", 0)
            if dock == 2:
                self.log_area.append("[INFO] Dock 成功，等待机器人反向退出")
            elif dock == 3:
                self.log_area.append("[WARN] Dock 失败/超时")

    def handle_ack(self, rtt_ms, payload):
        status = payload.get("status", 0)
        self.ack_label.setText(f"{rtt_ms:.1f} ms (status {status})")
        if status == 0:
            self.log_area.append(f"[ACK] OK, RTT={rtt_ms:.1f}ms")
        else:
            self.log_area.append(f"[ACK] status={status}, RTT={rtt_ms:.1f}ms")

    def update_stats(self):
        now = time.time()
        for msg, times in self.freq_stats.items():
            times = [t for t in times if now - t < 2.0]
            self.freq_stats[msg] = deque(times, maxlen=2000)
            freq = len(times) / 2.0
            target = TARGET_FREQ[msg]
            self.freq_labels[msg].setText(f"{freq:.1f} / {target} Hz")
            if freq < target * 0.8:
                self.freq_labels[msg].setStyleSheet("color: orange;")
            else:
                self.freq_labels[msg].setStyleSheet("color: green;")

        # 检测掉线
        if self.serial:
            for msg in TARGET_FREQ:
                last = self.last_recv_time.get(msg, 0)
                if now - last > 2.0:
                    self.log_area.append(f"[WARN] 2s 未收到 MSG 0x{msg:02X}")

# ----------------- 入口 -----------------
def main():
    app = QApplication(sys.argv)
    panel = ControlPanel()
    panel.show()
    sys.exit(app.exec())
    
if __name__ == "__main__":
    main()
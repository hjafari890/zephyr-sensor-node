import sys
import socket
import threading
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QHBoxLayout, QLabel, QSlider, QPushButton, QGroupBox
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QFont

class InputSimulator(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Biomedical Sensor-Node: Input Simulator")
        self.setGeometry(200, 200, 500, 600)
        self.setStyleSheet("background-color: #121212; color: #EEEEEE;")

        self.host = '127.0.0.1'
        self.port = 9998
        self.sock = None

        layout = QVBoxLayout()
        self.setLayout(layout)

        title = QLabel("Hardware-in-the-Loop Input Simulator")
        title.setFont(QFont("Inter", 16, QFont.Bold))
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        status_lbl = QLabel("This tool directly overwrites the physical sensor values inside the running Zephyr RTOS.")
        status_lbl.setWordWrap(True)
        status_lbl.setStyleSheet("color: #AAAAAA; margin-bottom: 20px;")
        layout.addWidget(status_lbl)

        # Create Sliders
        self.sliders = {}
        self.add_slider(layout, "Heart Rate (BPM)", "BPM", 40, 200, 70, " BPM")
        self.add_slider(layout, "Body Temp (°C x 10)", "TEMP", 300, 420, 365, " (x0.1 C)")
        self.add_slider(layout, "Motion / Accel Z", "ACCZ", 800, 1500, 981, " g-scale")
        self.add_slider(layout, "Humidity (%)", "HUM", 0, 100, 45, " %")
        self.add_slider(layout, "Ambient Light (Lux)", "LGT", 0, 1000, 300, " Lux")

        self.conn_btn = QPushButton("Connect to Renode (Port 9998)")
        self.conn_btn.setStyleSheet("background-color: #0055AA; font-weight: bold; padding: 15px; border-radius: 5px;")
        self.conn_btn.clicked.connect(self.connect_socket)
        layout.addWidget(self.conn_btn)

    def add_slider(self, parent, label_text, prefix, min_val, max_val, default_val, suffix):
        group = QGroupBox()
        vbox = QVBoxLayout()
        
        lbl = QLabel(f"{label_text}: {default_val}{suffix}")
        lbl.setFont(QFont("Inter", 12))
        
        slider = QSlider(Qt.Horizontal)
        slider.setMinimum(min_val)
        slider.setMaximum(max_val)
        slider.setValue(default_val)
        slider.setStyleSheet("QSlider::handle:horizontal { background: #00AAFF; border-radius: 5px; width: 15px; }")
        
        slider.valueChanged.connect(lambda val, l=lbl, p=prefix, s=suffix: self.on_value_changed(p, val, l, s))
        
        vbox.addWidget(lbl)
        vbox.addWidget(slider)
        group.setLayout(vbox)
        parent.addWidget(group)

    def on_value_changed(self, prefix, val, label, suffix):
        label.setText(f"{prefix.replace('ACCZ', 'Accel').replace('LGT', 'Light')}: {val}{suffix}")
        self.send_command(f"{prefix}:{val}\n")

    def connect_socket(self):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(2.0)
            self.sock.connect((self.host, self.port))
            self.conn_btn.setText("Connected!")
            self.conn_btn.setStyleSheet("background-color: #00AA00; font-weight: bold; padding: 15px; border-radius: 5px;")
            self.conn_btn.setEnabled(False)
        except Exception as e:
            self.conn_btn.setText("Connection Failed - Retry?")
            self.conn_btn.setStyleSheet("background-color: #AA0000; font-weight: bold; padding: 15px; border-radius: 5px;")

    def send_command(self, cmd):
        if self.sock:
            try:
                self.sock.sendall(cmd.encode('utf-8'))
                print(f"[TX] {cmd.strip()}")
            except:
                pass

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = InputSimulator()
    window.show()
    sys.exit(app.exec_())

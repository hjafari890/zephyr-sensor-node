import sys
import json
import socket
import threading
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget, QLabel
from PyQt5.QtCore import pyqtSignal, QObject, Qt
from PyQt5.QtGui import QFont, QColor
import pyqtgraph as pg

# Configure PyQtGraph global settings
pg.setConfigOption('background', '#151515')
pg.setConfigOption('foreground', '#E0E0E0')
pg.setConfigOptions(antialias=True)

class TelemetryReceiver(QObject):
    data_received = pyqtSignal(dict)

    def __init__(self, host='127.0.0.1', port=9999):
        super().__init__()
        self.host = host
        self.port = port
        self.running = True
        self.buffer = ""

    def connect_and_listen(self):
        import time
        while self.running:
            try:
                print(f"[*] Connecting to {self.host}:{self.port}...")
                with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                    s.settimeout(2.0)
                    s.connect((self.host, self.port))
                    s.settimeout(None)
                    print("[+] Connected successfully! Waiting for telemetry...")
                    while self.running:
                        chunk = s.recv(1024).decode('utf-8', errors='ignore')
                        if not chunk: break
                        self.buffer += chunk
                        while '\n' in self.buffer or '\r' in self.buffer:
                            # Handle both \n and \r\n from Zephyr UART
                            if '\n' in self.buffer:
                                line, self.buffer = self.buffer.split('\n', 1)
                            else:
                                line, self.buffer = self.buffer.split('\r', 1)
                                
                            print(f"[DEBUG] Received: {repr(line)}")
                            
                            if '[TELEMETRY]' in line:
                                start = line.find('{')
                                end = line.rfind('}')
                                if start != -1 and end != -1:
                                    json_str = line[start:end+1]
                                    try:
                                        self.data_received.emit(json.loads(json_str))
                                    except json.JSONDecodeError as e:
                                        print(f"[-] JSON Parse Error: {e}")
            except Exception as e: 
                print(f"[-] Connection error: {e}. Retrying in 2 seconds...")
                time.sleep(2)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Full Shield Biomedical Dashboard")
        self.setGeometry(100, 100, 1200, 800)
        
        main_widget = QWidget()
        self.setCentralWidget(main_widget)
        layout = QVBoxLayout(main_widget)
        
        # STATUS BAR
        self.status_label = QLabel("STATUS: OFFLINE")
        self.status_label.setFont(QFont("Inter", 24, QFont.Bold))
        self.status_label.setAlignment(Qt.AlignCenter)
        self.status_label.setStyleSheet("color: #888; background-color: #222; border-radius: 8px; padding: 10px;")
        layout.addWidget(self.status_label)
        
        # GRAPHS LAYOUT
        graphs_layout = pg.GraphicsLayoutWidget()
        layout.addWidget(graphs_layout)
        
        # 1. Heart Rate (MAX30102)
        self.p1 = graphs_layout.addPlot(title="Heart Rate (MAX30102)")
        self.p1.setLabel('left', 'BPM')
        self.hr_curve = self.p1.plot(pen=pg.mkPen(color=(255, 50, 50), width=3), fillLevel=40, fillBrush=(255,50,50,50))
        self.p1.setYRange(40, 200)
        
        # 2. Temperature (BME680/MLX90632)
        self.p2 = graphs_layout.addPlot(title="Body Temp (°C)")
        self.p2.setLabel('left', 'Celsius')
        self.temp_curve = self.p2.plot(pen=pg.mkPen(color=(255, 165, 0), width=3))
        self.p2.setYRange(30, 45)
        
        graphs_layout.nextRow()
        
        # 3. Motion (ADXL362 Z-Axis)
        self.p3 = graphs_layout.addPlot(title="Motion / Accel Z (ADXL362)")
        self.p3.setLabel('left', 'Magnitude')
        self.accel_curve = self.p3.plot(pen=pg.mkPen(color=(0, 255, 255), width=3))
        
        # 4. Environment (Humidity & Light)
        self.p4 = graphs_layout.addPlot(title="Env: Light (OPT4001) / Humidity (BME680)")
        self.p4.setLabel('left', 'Value')
        self.hum_curve = self.p4.plot(pen=pg.mkPen(color=(0, 255, 0), width=2), name="Humidity")
        self.light_curve = self.p4.plot(pen=pg.mkPen(color=(255, 255, 0), width=2), name="Light")
        
        # Data storage
        self.data = {'x': [], 'bpm': [], 'temp': [], 'accel': [], 'hum': [], 'light': []}
        self.tick = 0
        
        self.receiver = TelemetryReceiver()
        self.receiver.data_received.connect(self.update_ui)
        threading.Thread(target=self.receiver.connect_and_listen, daemon=True).start()

    def update_ui(self, p):
        s = p.get("status", "UNKNOWN")
        self.status_label.setText(f"STATUS: {s}")
        if s == "NORMAL": self.status_label.setStyleSheet("color: #0F0; background: #030; border: 2px solid #0F0;")
        elif s == "WARNING": self.status_label.setStyleSheet("color: #FD0; background: #330; border: 2px solid #FD0;")
        elif s == "ALARM": self.status_label.setStyleSheet("color: #F00; background: #300; border: 2px solid #F00;")
            
        self.tick += 1
        d = self.data
        d['x'].append(self.tick)
        d['bpm'].append(p.get("bpm", 0))
        d['temp'].append(p.get("temp_c", 0) / 10.0) # convert 365 to 36.5
        d['accel'].append(p.get("accel_mag", 0))
        d['hum'].append(p.get("humidity", 0))
        d['light'].append(p.get("light", 0))
        
        if len(d['x']) > 50:
            for k in d: d[k] = d[k][-50:]
            
        self.hr_curve.setData(d['x'], d['bpm'])
        self.temp_curve.setData(d['x'], d['temp'])
        self.accel_curve.setData(d['x'], d['accel'])
        self.hum_curve.setData(d['x'], d['hum'])
        self.light_curve.setData(d['x'], d['light'])

if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())

import socket
import json
import time

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(30)
connected = False
while not connected:
    try:
        s.connect(('127.0.0.1', 9999))
        connected = True
    except:
        time.sleep(1)

print('=============================================')
print('        TERMINAL TELEMETRY DASHBOARD         ')
print('=============================================')

buffer = ''
count = 0
while count < 5:
    try:
        chunk = s.recv(1024).decode('utf-8', errors='ignore')
        if not chunk: break
        buffer += chunk
        while '\n' in buffer or '\r' in buffer:
            if '\n' in buffer:
                line, buffer = buffer.split('\n', 1)
            else:
                line, buffer = buffer.split('\r', 1)
            if '[TELEMETRY]' in line:
                start = line.find('{')
                end = line.rfind('}')
                if start != -1 and end != -1:
                    data = json.loads(line[start:end+1])
                    print(f"| STATUS: {data.get('status'):<7} | UPTIME: {data.get('uptime_ms'):<6} | BPM: {data.get('bpm'):<3} | TEMP: {data.get('temp_c')/10.0}C |")
                    count += 1
    except Exception as e:
        print('Error:', e)
        break
print('=============================================')

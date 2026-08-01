import socket
s = socket.socket()
s.settimeout(15)
s.connect(('127.0.0.1', 9999))
print('Connected to telemetry socket.')
buf = ''
for _ in range(10):
    try:
        chunk = s.recv(1024).decode('utf-8', errors='ignore')
        if not chunk: break
        buf += chunk
        if '[TELEMETRY]' in buf:
            print('JSON FOUND IN STREAM:', buf[buf.find('{'):buf.rfind('}')+1])
            break
    except:
        pass

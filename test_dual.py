import socket
s = socket.socket()
s.settimeout(5)
s.connect(('127.0.0.1', 9999))
print('Connected')
print(s.recv(1024).decode('utf-8', errors='ignore'))

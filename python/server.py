#!/usr/bin/env python3

import socket
from time import sleep
#import exploration

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 61924        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(2)
    conn, addr = s.accept()
    with conn:
        print('Connected by', addr)
        while True:
            data = conn.recv(1024)
            print("Received from ", addr, data)
            
            
            conn.sendall(data)
            sleep(1)
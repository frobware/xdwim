#!/usr/bin/env python

import sys
import os
import socket

server_address = '/tmp/xdwim.sock'
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

rc = 1

try:
    sock.connect(server_address)
    sock.sendall(sys.argv[1] + "\n")
    data = sock.recv(32)
    if data == "success":
        rc = 0
finally:
    sock.close()

exit(rc)

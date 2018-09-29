#!/usr/bin/env python

import sys
import os
import socket

server_address = '/tmp/xdwim.sock'
sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)

try:
    message = sys.argv[1] + "\n"
    print('sending "%s"' % message)
    sock.connect(server_address)
    sock.sendall(message)
    amount_received = 0
    amount_expected = len(message)
    while amount_received < amount_expected:
        data = sock.recv(1024)
        amount_received += len(data)
        print(amount_received, 'received "%s"' % data)
finally:
    print('closing socket')
    sock.close()

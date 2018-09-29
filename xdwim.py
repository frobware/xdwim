#!/usr/bin/env python

import sys
import os
import SocketServer

from xdo import Xdo

def switch_to_window(classname):
    windows = xdo.search_windows(winclassname=classname.split(".")[0],
                                 desktop=xdo.get_current_desktop(),
                                 only_visible=True,
                                 require=True)
    print("classname", len(windows), windows)
    if not windows:
        return
    topmost_window = windows.pop()
    print("classname topmost_window", topmost_window)
    active_window = xdo.get_active_window()
    print("active_window", active_window)
    if topmost_window == active_window and len(windows) >= 1:
        topmost_window = windows.pop()
        print("classname, selecting next window", topmost_window)
    if topmost_window != active_window:
        print("activating", topmost_window)
        xdo.activate_window(topmost_window)
        xdo.focus_window(topmost_window)

class Handler(SocketServer.StreamRequestHandler):
    def handle(self):
        self.data = self.rfile.readline().strip()
        print(self.data)
        if self.data:
            switch_to_window(self.data.split()[0])
            self.request.sendall(self.data + "\n")
            self.finish()
        return

server_address = '/tmp/xdwim.sock'

if os.path.exists(server_address):
    os.remove(server_address)

xdo = Xdo()
svr = SocketServer.UnixStreamServer(server_address, Handler, True)

try:
    svr.serve_forever()
except (KeyboardInterrupt, SystemExit):
    os.remove(server_address)
    pass

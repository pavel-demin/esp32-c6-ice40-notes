import socket
from socket import socket, AF_INET, SOCK_STREAM, SHUT_RDWR

import numpy as np


class PyhubESP:
    def __init__(self, host="192.168.4.1", port=1001):
        self.address = (host, port)
        self.socket = None

    def start(self):
        if self.socket:
            return
        self.socket = socket(AF_INET, SOCK_STREAM)
        self.socket.connect(self.address)

    def stop(self):
        if self.socket is None:
            return
        self.socket.shutdown(SHUT_RDWR)
        self.socket.close()
        self.socket = None

    def write(self, data, port=0, addr=0):
        if self.socket is None:
            return
        view = data.view(np.uint32)
        for part in np.split(view, np.arange(1024, view.size, 1024)):
            size = part.size
            command = np.uint32([1 << 30 | (size - 1) << 20 | (port & 0x7) << 17 | addr & 0x1FFFF])
            addr += part.size
            self.socket.sendall(command.tobytes())
            self.socket.sendall(part.tobytes())

    def read(self, data, port=1, addr=0):
        if self.socket is None:
            return
        view = data.view(np.uint32)
        for part in np.split(view, np.arange(1048576, view.size, 1048576)):
            view = part.view(np.uint8)
            size = part.size
            incr = np.arange(0, size, 1024, np.uint32)
            rlen = np.full_like(incr, 1023)
            mod = size % 1024
            if mod > 0:
                rlen[-1] = mod - 1
            command = rlen << 20 | (port & 0x7) << 17 | (addr + incr) & 0x1FFFF
            self.socket.sendall(command.tobytes())
            offset = 0
            limit = view.size
            while offset < limit:
                buffer = self.socket.recv(limit - offset)
                buffer = np.frombuffer(buffer, np.uint8)
                size = buffer.size
                view[offset : offset + size] = buffer
                offset += size
            addr += part.size

    def edge(self, data, mask, positive=True, addr=0):
        if self.socket is None:
            return data
        command = np.uint32(1 << 30 | addr & 0x1FFFF)
        lo = data & ~mask
        hi = data | mask
        if positive:
            sequence = np.uint32([command, lo, command, hi])
            result = hi
        else:
            sequence = np.uint32([command, hi, command, lo])
            result = lo
        self.socket.sendall(sequence.tobytes())
        return result

    def program(self, path):
        if self.socket is None:
            return
        data = np.fromfile(path, np.uint8)
        command = np.uint32([1 << 31 | data.size])
        self.socket.sendall(command.tobytes())
        self.socket.sendall(data.tobytes())

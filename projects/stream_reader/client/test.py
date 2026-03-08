from pyhubio import PyhubESP
import numpy as np
import time

io = PyhubESP("192.168.4.1")

io.start()
io.program("stream_reader.bin")

size = 2**26

input = np.arange(0, size, 1, np.uint32)

output = np.zeros(size, np.uint32)

f = open("input.dat", "wb")
f.write(input.tobytes())
f.close()

f = open("output.dat", "wb")

io.write(np.uint32([149]), port=0, addr=1)
io.write(np.uint32([1]), port=0, addr=0)

before = time.time()

offset = 0
view = output.view(np.uint8)
limit = view.size
while offset < limit:
    buffer = io.socket.recv(limit - offset)
    buffer = np.frombuffer(buffer, np.uint8)
    size = buffer.size
    view[offset : offset + size] = buffer
    offset += size

after = time.time()

io.write(np.uint32([0]), port=0, addr=0)

io.stop()

print(output)

f.write(output.tobytes())
f.close()

duration = after - before
speed = input.view(np.uint8).size / duration

print("time, s:", np.format_float_positional(duration, precision=1))
print("speed, MB/s:", np.format_float_positional(speed / 2**20, precision=1))

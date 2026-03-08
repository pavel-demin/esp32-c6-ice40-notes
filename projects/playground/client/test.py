from pyhubio import PyhubESP
import numpy as np
import time

io = PyhubESP("192.168.4.1")

io.start()
io.program("playground.bin")

buffer = np.zeros(2 * 2**20, np.uint32)

before = time.time()
io.read(buffer, port=2, addr=0)
after = time.time()

duration = after - before
speed = buffer.view(np.uint8).size / duration

print("time, s:", np.format_float_positional(duration, precision=1))
print("speed, MB/s:", np.format_float_positional(speed / 2**20, precision=1))

io.stop()

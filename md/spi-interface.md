# SPI interface

## Requirements

All applications in this repository have a structure similar to the one shown in the following diagram:

![Application structure](/img/application-structure.png)

To control, monitor and communicate with all parts of the applications, the following items are required:

- configuration registers
- status registers
- AXI4-Stream interfaces
- BRAM interfaces

## SPI interface

The SPI interface consists of two FIFO buffers and two AXI4-Stream interfaces. One of the FIFO buffers is used for data received from the microcontroller and the other FIFO buffer is used for data to be sent to the microcontroller.

The corresponding Verilog code can be found in [modules/axis_spi.v]($source$/modules/axis_spi.v).

## Hub interface

The hub interface consists of two AXI4-Stream interfaces used to communicate with the SPI interface and all other required registers and interfaces connected to different parts of the applications.

The corresponding Verilog code can be found in [modules/axis_hub.v]($source$/modules/axis_hub.v).

## Communication protocol

Communication packets sent from the microcontroller to the hub interface consist of a 32-bit command followed by 0-1024 32-bit data words.

The command consists of 32 bits containing the following information:

| information  | bits    |
| ------------ | ------- |
| port address | 0 - 16  |
| hub address  | 17 - 19 |
| burst length | 20 - 29 |
| write/read#  | 30      |

If the write/read# bit is 1, then the next number of 32-bit data words corresponding to the burst length will be interpreted as data and written to consecutive addresses starting from the address specified in the command.

If the write/read# bit is 0, then no data is expected. Instead, the number of 32-bit data words corresponding to the burst length will be read from consecutive addresses starting from the address specified in the command.

The hub address is used to select one of the hub ports:

| hub port        | hub address |
| --------------- | ----------- |
| config register | 0           |
| status register | 1           |
| interface 0     | 2           |
| interface 1     | 3           |
| interface 2     | 4           |
| interface 3     | 5           |
| interface 4     | 6           |
| interface 5     | 7           |

The port address is used to communicate with the configuration registers, status registers and BRAM modules connected to the BRAM interfaces.

The data sent from the SPI interface to the microcontroller consist of a number of 32-bit data words corresponding to the burst length in the commands with the write/read# bit set to 0.

## Software

A Python library is used to communicate with configuration registers, status registers, AXI4-Stream and BRAM interfaces.

The Python code of this library can be found in [pyhubio](https://github.com/pavel-demin/pyhubio).

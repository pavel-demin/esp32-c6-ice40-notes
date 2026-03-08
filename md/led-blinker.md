# LED blinker

## Introduction

For my experiments with ESP32-C6 and iCE40, I would like to have the following development environment:

- recent version of the [Debian distribution](https://www.debian.org/releases/bookworm) on the development machine
- basic project with all the peripherals connected
- mostly command-line tools
- shallow directory structure

Here is how I set it all up.

## Pre-requirements

My development machine has the following installed:

- [Debian](https://www.debian.org/releases/trixie) 13 (amd64)
- [Espressif IoT Development Framework](https://idf.espressif.com) 5.5
- [Project Icestorm tools](https://prjicestorm.readthedocs.io)

## Source code

The source code is available at

<https://github.com/pavel-demin/esp32-c6-ice40-notes>

This repository contains the following components:

- [Makefile]($source$/Makefile) that builds everything (almost)
- [cfg]($source$/cfg) directory with constraints and configuration files
- [modules]($source$/modules) directory with modules written in Verilog
- [projects]($source$/projects) directory with projects written in Verilog

## Getting started

Cloning the source code repository:

```bash
git clone https://github.com/pavel-demin/esp32-c6-ice40-notes
cd esp32-c6-ice40-notes
```

Building `led_blinker.bin`:

```bash
make NAME=led_blinker bin
```

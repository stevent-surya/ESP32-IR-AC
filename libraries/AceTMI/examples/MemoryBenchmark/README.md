# Memory Benchmark

The `MemoryBenchmark.ino` collects the amount of flash and static memory
consumed by different implementations in the AceTMI library.

It compiles various code snippets which are controlled by the `FEATURE` macro
flag. The `collect.sh` edits this `FEATURE` flag programmatically, then runs the
Arduino IDE compiler on the program, and extracts the flash and static memory
usage into a text file (e.g. `nano.txt`).

The numbers shown below should be considered to be rough estimates. It is often
difficult to separate out the code size of the library from the overhead imposed
by the runtime environment of the processor. For example, it often seems like
the ESP8266 allocates flash memory in blocks of a certain quantity, so the
calculated flash size can jump around in unexpected ways.

**Version**: AceTMI v0.6

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules for several microcontrollers:

```
$ make benchmarks
```
produces the following files:

```
attiny.txt
nano.txt
micro.txt
stm32.txt
esp8266.txt
esp32.txt
teensy32.txt
```

The `generate_table.awk` program reads one of `*.txt` files and prints out an
ASCII table that can be directly embedded into this README.md file. For example
the following command produces the table in the Nano section below:

```
$ ./generate_table.awk < nano.txt
```

Fortunately, we no longer need to run `generate_table.awk` for each `*.txt`
file. The process has been automated using the `generate_readme.py` script which
will be invoked by the following command:
```
$ make README.md
```

## Library Size Changes

**v0.2**

* Initial MemoryBenchmark results.

## Results

The following shows the flash and static memory sizes of the `MemoryBenchmark`
program for various `LedModule` configurations and various Writer classes.

* `SimpleTmi1637Interface`: implementation using `digitalWrite()`
* `SimpleTmi1637FastInterface`: implementation using `digitalWriteFast()`

### ATtiny85

* 8MHz ATtiny85
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SpenceKonde/ATTinyCore 1.5.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    260/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          |    924/   14 |   664/    3 |
| SimpleTmi1637FastInterface      |    374/   11 |   114/    0 |
| SimpleTmi1638Interface          |    844/   15 |   584/    4 |
| SimpleTmi1638FastInterface      |    362/   11 |   102/    0 |
+--------------------------------------------------------------+

```

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.4

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |    456/   11 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          |   1200/   14 |   744/    3 |
| SimpleTmi1637FastInterface      |    638/   11 |   182/    0 |
| SimpleTmi1638Interface          |   1108/   15 |   652/    4 |
| SimpleTmi1638FastInterface      |    590/   11 |   134/    0 |
+--------------------------------------------------------------+

```

### Sparkfun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |   3472/  151 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          |   4292/  154 |   820/    3 |
| SimpleTmi1637FastInterface      |   3654/  151 |   182/    0 |
| SimpleTmi1638Interface          |   4200/  155 |   728/    4 |
| SimpleTmi1638FastInterface      |   3606/  151 |   134/    0 |
+--------------------------------------------------------------+

```

### STM32 Blue Pill

* STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.2.0

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  21880/ 3540 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          |  24532/ 3564 |  2652/   24 |
| SimpleTmi1638Interface          |  24384/ 3564 |  2504/   24 |
+--------------------------------------------------------------+

```

### ESP8266

* NodeMCU 1.0, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 260089/27892 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          | 261361/27992 |  1272/  100 |
| SimpleTmi1638Interface          | 261217/27992 |  1128/  100 |
+--------------------------------------------------------------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 2.0.2

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        | 204501/16060 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          | 207489/16228 |  2988/  168 |
| SimpleTmi1638Interface          | 207305/16228 |  2804/  168 |
+--------------------------------------------------------------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
+--------------------------------------------------------------+
| functionality                   |  flash/  ram |       delta |
|---------------------------------+--------------+-------------|
| baseline                        |  10216/ 4152 |     0/    0 |
|---------------------------------+--------------+-------------|
| SimpleTmi1637Interface          |  11544/ 4156 |  1328/    4 |
| SimpleTmi1638Interface          |  11220/ 4156 |  1004/    4 |
+--------------------------------------------------------------+

```


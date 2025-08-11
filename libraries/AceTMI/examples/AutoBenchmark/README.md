# AutoBenchmark

This program measures the performance of various implementations of the TM1637
protocol in the AceTMI library: `SimpleTmi1637Interface` and
`SimpleTmi1637FastInterface`.

**Version**: AceTMI v0.6

**DO NOT EDIT**: This file was auto-generated using `make README.md`.

## Dependencies

This program depends on the following libraries:

* [AceCommon](https://github.com/bxparks/AceCommon)
* [AceTMI](https://github.com/bxparks/AceTMI)

On AVR processors, one of the following libraries is required to run the
`digitalWriteFast()` versions of the low-level drivers:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast
* https://github.com/NicksonYap/digitalWriteFast

## How to Generate

This requires the [AUniter](https://github.com/bxparks/AUniter) script
to execute the Arduino IDE programmatically.

The `Makefile` has rules to generate the `*.txt` results file for several
microcontrollers that I usually support, but the `$ make benchmarks` command
does not work very well because the USB port of the microcontroller is a
dynamically changing parameter. I created a semi-automated way of collect the
`*.txt` files:

1. Connect the microcontroller to the serial port. I usually do this through a
USB hub with individually controlled switch.
2. Type `$ auniter ports` to determine its `/dev/ttyXXX` port number (e.g.
`/dev/ttyUSB0` or `/dev/ttyACM0`).
3. If the port is `USB0` or `ACM0`, type `$ make nano.txt`, etc.
4. Switch off the old microontroller.
5. Go to Step 1 and repeat for each microcontroller.

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

The CPU times below are given in microseconds. The "samples" column is the
number of `TimingStats::update()` calls that were made.

## CPU Time Changes

**v0.2:**
* Initial version extracted from AceSegment/AutoBenchmark.

## Results

The following tables show the number of microseconds taken by:

* `SimpleTmi1637Interface`
* `SimpleTmi1637FastInterface`
* `SimpleTmi1638Interface`
* `SimpleTmi1638FastInterface`

On AVR processors, the "fast" options are available using one of the
digitalWriteFast libraries whose `digitalWriteFast()` functions can be up to 50X
faster if the `pin` number and `value` parameters are compile-time constants. In
addition, the `digitalWriteFast` functions reduce flash memory consumption by
500-700 bytes compared to their non-fast equivalents.

### Arduino Nano

* 16MHz ATmega328P
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Arduino AVR Boards 1.8.4
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1637FastInterface<4, 5, 100>): 1
sizeof(SimpleTmi1638Interface): 4
sizeof(SimpleTmi1638FastInterface<4, 5, 6, 1>): 1

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   752/  781/  836 |     41.0 |
| SimpleTmi1637FastInterface,1us          |    92/   95/  104 |    336.8 |
| SimpleTmi1638Interface,1us              |   616/  635/  688 |     50.4 |
| SimpleTmi1638FastInterface,1us          |    84/   86/   92 |    372.1 |
+-----------------------------------------+-------------------+----------+

```

### SparkFun Pro Micro

* 16 MHz ATmega32U4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* SparkFun AVR Boards 1.1.13
* `micros()` has a resolution of 4 microseconds

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1637FastInterface<4, 5, 100>): 1
sizeof(SimpleTmi1638Interface): 4
sizeof(SimpleTmi1638FastInterface<4, 5, 6, 1>): 1

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   756/  761/  764 |     42.0 |
| SimpleTmi1637FastInterface,1us          |    84/   88/   96 |    363.6 |
| SimpleTmi1638Interface,1us              |   756/  764/  768 |     41.9 |
| SimpleTmi1638FastInterface,1us          |    76/   79/   88 |    405.1 |
+-----------------------------------------+-------------------+----------+

```

### STM32

* STM32 "Blue Pill", STM32F103C8, 72 MHz ARM Cortex-M3
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* STM32duino 2.2.0

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1638Interface): 4

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   877/  882/  910 |     36.3 |
| SimpleTmi1638Interface,1us              |   433/  435/  438 |     73.6 |
+-----------------------------------------+-------------------+----------+

```

### ESP8266

* NodeMCU 1.0 clone, 80MHz ESP8266
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP8266 Boards 3.0.2

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1638Interface): 4

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   375/  377/  415 |     84.9 |
| SimpleTmi1638Interface,1us              |   334/  335/  354 |     95.5 |
+-----------------------------------------+-------------------+----------+

```

### ESP32

* ESP32-01 Dev Board, 240 MHz Tensilica LX6
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* ESP32 Boards 2.0.2

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1638Interface): 4

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   230/  231/  238 |    138.5 |
| SimpleTmi1638Interface,1us              |   171/  173/  180 |    185.0 |
+-----------------------------------------+-------------------+----------+

```

### Teensy 3.2

* 96 MHz ARM Cortex-M4
* Arduino IDE 1.8.19, Arduino CLI 0.19.2
* Teensyduino 1.56
* Compiler options: "Faster"

```
Sizes of Objects:
sizeof(SimpleTmi1637Interface): 3
sizeof(SimpleTmi1638Interface): 4

CPU:
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   177/  177/  181 |    180.8 |
| SimpleTmi1638Interface,1us              |   151/  151/  154 |    211.9 |
+-----------------------------------------+-------------------+----------+

```


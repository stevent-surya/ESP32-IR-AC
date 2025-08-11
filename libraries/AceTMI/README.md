# AceTMI

[![Validation](https://github.com/bxparks/AceTMI/actions/workflows/validation.yml/badge.svg)](https://github.com/bxparks/AceTMI/actions/workflows/validation.yml)

Interface classes for communicating with a TM1637 or TM1638 LED controller chip
on Arduino platforms. The code was initially part of the
[AceSegment](https://github.com/bxparks/AceSegment) library, but was extracted
into a separate library for consistency with the
[AceWire](https://github.com/bxparks/AceWire) and
[AceSPI](https://github.com/bxparks/AceSPI) libraries. It provides the following
implementations:

* `SimpleTmi1637Interface`
    * Implements the TM1637 protocol using `digitalWrite()`.
* `SimpleTmi1637FastInterface`
    * Implements the TM1637 protocol using `digitalWriteFast()`.
    * 4X less flash memory than `SimpleTmi1637Interface` (182 bytes versus
      820 bytes).
    * 9X faster than `SimpleTmi1637Interface` (363 kbps versus 42 kbps).
* `SimpleTmi1638Interface`
    * Implements the TM1638 protocol using `digitalWrite()`.
* `SimpleTmi1638FastInterface`
    * Implements the TM1638 protocol using `digitalWriteFast()`.
    * 5X less flash memory than `SimpleTmi1638Interface` (102 bytes
      versus 584 bytes).
    * 10X faster than `SimpleTmi1638Interface` (405 kbps versus 42 kbps).

The "TMI" acronym was invented by this library to name the protocol used by the
TM1637 or the TM1638 chip. It stands for "Titan Micro Interface", named after
Titan Micro Electronics which manufactures these chips. The TM1637 protocol is
electrically similar to I2C, but different enough that we cannot use the
`<Wire.h>` I2C library. The TM1638 protocol is electrically similar to SPI but
different enough that we cannot use the `<SPI.h>` library.

The protocol implemented by this library works only for the TM1637 or the TM1638
chips as far as I know. Most people will want to use the `Tm1637Module` or the
`Tm1638Module` class in the [AceSegment](https://github.com/bxparks/AceSegment)
library instead of using this low-level library.

The library uses C++ templates to achieve minimal runtime overhead. In more
technical terms, the library provides compile-time polymorphism instead of
runtime polymorphism to avoid the overhead of the `virtual` keyword.

**Version**: 0.6 (2022-03-01)

**Changelog**: [CHANGELOG.md](CHANGELOG.md)

**See Also**:
* https://github.com/bxparks/AceSPI
* https://github.com/bxparks/AceWire

## Table of Contents

* [Installation](#Installation)
    * [Source Code](#SourceCode)
    * [Dependencies](#Dependencies)
* [Documentation](#Documentation)
* [Usage](#Usage)
    * [Include Header and Namespace](#HeaderAndNamespace)
    * [Unified Interface](#UnifiedInterface)
    * [SimpleTmi1637Interface](#SimpleTmi1637Interface)
    * [SimpleTmi1637FastInterface](#SimpleTmi1637FastInterface)
    * [SimpleTmi1638Interface](#SimpleTmi1638Interface)
    * [SimpleTmi1638FastInterface](#SimpleTmi1638FastInterface)
    * [Storing Interface Objects](#StoringInterfaceObjects)
* [Resource Consumption](#ResourceConsumption)
    * [Flash And Static Memory](#FlashAndStaticMemory)
    * [CPU Cycles](#CpuCycles)
* [System Requirements](#SystemRequirements)
    * [Hardware](#Hardware)
    * [Tool Chain](#ToolChain)
    * [Operating System](#OperatingSystem)
* [License](#License)
* [Feedback and Support](#FeedbackAndSupport)
* [Authors](#Authors)

<a name="Installation"></a>
## Installation

The latest stable release is available in the Arduino IDE
Library Manager. Search for "AceTMI". Click install.

The development version can be installed by cloning the
[GitHub repository](https://github.com/bxparks/AceTMI), checking out the
default `develop` branch, then manually copying over to or symlinking from the
`./libraries` directory used by the Arduino IDE. (The result is a directory
or link named `./libraries/AceTMI`.)

The `master` branch contains the stable releases.

<a name="SourceCode"></a>
### Source Code

The source files are organized as follows:
* `src/AceTMI.h` - main header file
* `src/ace_tmi/` - implementation files
* `docs/` - contains the doxygen docs and additional manual docs

<a name="Dependencies"></a>
### Dependencies

The main `AceTMI.h` does not depend any external libraries.

The "Fast" version (`SimpleTmi1637FastInterface.h`)
depends on one of the digitalWriteFast libraries, for example:

* https://github.com/watterott/Arduino-Libs/tree/master/digitalWriteFast
* https://github.com/NicksonYap/digitalWriteFast

<a name="Documentation"></a>
## Documentation

* this `README.md` file.
* [Doxygen docs](https://bxparks.github.io/AceTMI/html)
    * On Github pages.
* Examples:
    * https://github.com/bxparks/AceSegment/tree/develop/examples/HelloTm1637
    * https://github.com/bxparks/AceSegment/tree/develop/examples/HelloTm1638
    * https://github.com/bxparks/AceSegment/tree/develop/examples/Tm1637Demo
    * https://github.com/bxparks/AceSegment/tree/develop/examples/Tm1637DualDemo
    * https://github.com/bxparks/AceSegment/tree/develop/examples/Tm1638Demo

<a name="Usage"></a>
## Usage

<a name="HeaderAndNamespace"></a>
### Include Header and Namespace

In many cases, only a single header file `AceTMI.h` is required to use this
library. To prevent name clashes with other libraries that the calling code may
use, all classes are defined in the `ace_tmi` namespace. To use the code without
prepending the `ace_tmi::` prefix, use the `using` directive:

```C++
#include <Arduino.h>
#include <AceTMI.h>
using ace_tmi::SimpleTmi1637Interface;
```

The "Fast" versions are not included automatically by `AceTMI.h` because they
work only on AVR processors and they depend on a `<digitalWriteFast.h>`
library. To use the "Fast" versions, use something like the following:'

```C++
#include <Arduino.h>
#include <AceTMI.h>

#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1637FastInterface.h>
  using ace_tmi::SimpleTmi1637FastInterface;
#endif
```

<a name="UnifiedInterface"></a>
### Unified Interface

The classes in this library provide the following unified interface for handling
communication with the TM1637 chip. Downstream classes can code against this
generic API using C++ templates so that different implementations can be
selected at compile-time.

```C++
class XxxInterface {
  public:
    void begin() const;
    void end() const;

    void startCondition() const;
    void stopCondition() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

The TM1638 interface classes follow a similar pattern:

```C++
class Xxx1638Interface {
  public:
    void begin() const;
    void end() const;

    void beginTransaction() const;
    void endTransaction() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

Notice that the classes in this library do *not* inherit from a common interface
with virtual functions. This saves several hundred bytes of flash memory on
8-bit AVR processors by avoiding the dynamic dispatch, and often allows the
compiler to optimize away the overhead of calling the methods in this library so
that the function call is made directly to the underlying implementation. The
reduction of flash memory consumption is especially large for classes that use
the digitalWriteFast libraries which use compile-time constants for pin numbers.
The disadvantage is that this library is harder to use because these classes
require the downstream classes to be implemented using C++ templates.

The `read()` method reads the keypad scan data from the controller chip:

* TM1637 supports 2x8 buttons.
    * A single 1 byte encodes the segment lines and the key lines.
    * Only a single button can be pressed at the same time.
* TM1638 supports 3x8 buttons.
    * Four bytes are required to be obtained using `read()`.
    * Multiple buttons can be pressed at the same time.

<a name="SimpleTmi1637Interface"></a>
### SimpleTmi1637Interface

The `SimpleTmi1637Interface` can be used like this to communicate with a TM1637
controller chip. It looks like this:

```C++
class SimpleTmi1637Interface {
  public:
    explicit SimpleTmi1637Interface(
        uint8_t dioPin,
        uint8_t clkPin,
        uint8_t delayMicros
    );

    void begin() const;
    void end() const;

    void startCondition() const;
    void stopCondition() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

It is configured and used by the calling code `MyClass` like this:

```C++
#include <Arduino.h>
#include <AceTMI.h>
using ace_tmi::SimpleTmi1637Interface;

template <typename T_TMII>
class MyClass {
  public:
    explicit MyClass(T_TMII& tmiInterface)
        : mTmiInterface(tmiInterface)
    {...}

    void sendData() {
      // Set addressing mode.
      mTmiInterface.startCondition();
      mTmiInterface.write(addressMode);
      mTmiInterface.stopCondition();

      // Send data bytes.
      mTmiInterface.startCondition();
      mTmiInterface.write(otherCommand);
      [...]
      mTmiInterface.stopCondition();

      // Set brightness.
      mTmiInterface.startCondition();
      mTmiInterface.write(brightness);
      mTmiInterface.stopCondition();
    }

  private:
    T_TMII mTmiInterface; // copied by value
};

const uint8_t CLK_PIN = 8;
const uint8_t DIO_PIN = 9;
const uint8_t DELAY_MICROS = 100;

using TmiInterface = SimpleTmi1637Interface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, DELAY_MICROS);
MyClass<TmiInterface> myClass(tmiInterface);

void setup() {
  tmiInterface.begin();
  ...
}
```

The `using` statement is the C++11 version of a `typedef` that defines
`TmiInterface`. It is not strictly necessary here, but it allows the same
pattern to be used for the more complicated examples below.

The `T_TMII` template parameter contains a `T_` prefix to avoid name collisions
with too many `#define` macros defined in the global namespace on Arduino
platforms. The double `II` contains 2 `Interface`, the first referring to the
TM1637 protocol, and the second referring to classes in this library.

<a name="SimpleTmi1637FastInterface"></a>
### SimpleTmi1637FastInterface

The `SimpleTmi1637FastInterface` is identical to `SimpleTmi1637Interface` except
that it uses `digitalWriteFast()`. It looks like this:

```C++
template <
    uint8_t T_DIO_PIN,
    uint8_t T_CLK_PIN,
    uint8_t T_DELAY_MICROS
>
class SimpleTmi1637FastInterface {
  public:
    explicit SimpleTmi1637FastInterface() = default;

    void begin() const;
    void end() const;

    void startCondition() const;
    void stopCondition() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

It is configured and used by the calling code `MyClass` like this:

```C++
#include <Arduino.h>
#include <AceTMI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1637FastInterface.h>
  using ace_tmi::SimpleTmi1637Interface;
#endif

const uint8_t CLK_PIN = 8;
const uint8_t DIO_PIN = 9;
const uint8_t DELAY_MICROS = 100;

template <typename T_TMII>
class MyClass {
  // Exactly same as above.
};

using TmiInterface = SimpleTmi1637FastInterface<DIO_PIN, CLK_PIN, DELAY_MICROS>;
TmiInterface tmiInterface;
MyClass<TmiInterface> myClass(tmiInterface);

void setup() {
  tmiInterface.begin();
  ...
}
```

<a name="SimpleTmi1638Interface"></a>
### SimpleTmi1638Interface

The `SimpleTmi1638Interface` can be used like this to communicate with a TM1637
controller chip. It looks like this:

```C++
class SimpleTmi1638Interface {
  public:
    explicit SimpleTmi1638Interface(
        uint8_t dioPin,
        uint8_t clkPin,
        uint8_t stbPin,
        uint8_t delayMicros
    );

    void begin() const;
    void end() const;

    void beginTransaction() const;
    void endTransaction() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

It is configured and used by the calling code `MyClass` like this:

```C++
#include <Arduino.h>
#include <AceTMI.h>
using ace_tmi::SimpleTmi1638Interface;

template <typename T_TMII>
class MyClass {
  public:
    explicit MyClass(T_TMII& tmiInterface)
        : mTmiInterface(tmiInterface)
    {...}

    void sendData() {
      // Set addressing mode.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(dataCommand);
      mTmiInterface.endTransaction();

      // Send data bytes.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(addressCommand);
      [...]
      mTmiInterface.endTransaction();

      // Set brightness.
      mTmiInterface.beginTransaction();
      mTmiInterface.write(brightness);
      mTmiInterface.endTransaction();
    }

  private:
    T_TMII mTmiInterface; // copied by value
};

const uint8_t CLK_PIN = 8;
const uint8_t DIO_PIN = 9;
const uint8_t STB_PIN = 10;
const uint8_t DELAY_MICROS = 1;

using TmiInterface = SimpleTmi1638Interface;
TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, DELAY_MICROS);
MyClass<TmiInterface> myClass(tmiInterface);

void setup() {
  tmiInterface.begin();
  ...
}
```

The `using` statement is the C++11 version of a `typedef` that defines
`TmiInterface`. It is not strictly necessary here, but it allows the same
pattern to be used for the more complicated examples below.

The `T_TMII` template parameter contains a `T_` prefix to avoid name collisions
with too many `#define` macros defined in the global namespace on Arduino
platforms. The double `II` contains 2 `Interface`, the first referring to the
TM1637 protocol, and the second referring to classes in this library.

<a name="SimpleTmi1638FastInterface"></a>
### SimpleTmi1638FastInterface

The `SimpleTmi1638FastInterface` is identical to `SimpleTmi1638Interface` except
that it uses `digitalWriteFast()`. It looks like this:

```C++
template <
    uint8_t T_DIO_PIN,
    uint8_t T_CLK_PIN,
    uint8_t T_STB_PIN,
    uint8_t T_DELAY_MICROS
>
class SimpleTmi1638FastInterface {
  public:
    explicit SimpleTmi1638FastInterface() = default;

    void begin() const;
    void end() const;

    void beginTransaction() const;
    void endTransaction() const;
    uint8_t write(uint8_t data) const;
    uint8_t read() const;
};
```

It is configured and used by the calling code `MyClass` like this:

```C++
#include <Arduino.h>
#include <AceTMI.h>
#if defined(ARDUINO_ARCH_AVR)
  #include <digitalWriteFast.h>
  #include <ace_tmi/SimpleTmi1638FastInterface.h>
  using ace_tmi::SimpleTmi1638Interface;
#endif

const uint8_t CLK_PIN = 8;
const uint8_t DIO_PIN = 9;
const uint8_t STB_PIN = 10;
const uint8_t DELAY_MICROS = 1;

template <typename T_TMII>
class MyClass {
  // Exactly same as above.
};

using TmiInterface = SimpleTmi1638FastInterface<
    DIO_PIN, CLK_PIN, STB_PIN, DELAY_MICROS>;
TmiInterface tmiInterface;
MyClass<TmiInterface> myClass(tmiInterface);

void setup() {
  tmiInterface.begin();
  ...
}
```

<a name="StoringInterfaceObjects"></a>
### Storing Interface Objects

In the above examples, the `MyClass` object holds the `T_TMII` interface object
**by value**. In other words, the interface object is copied into the `MyClass`
object. This is efficient because interface objects are very small in size, and
copying them by-value avoids an extra level of indirection when they are used
inside the `MyClass` object.

The alternative is to save the `T_TMII` object **by reference** like this:

```C++
template <typename T_TMII>
class MyClass {
  public:
    explicit MyClass(T_TMII& tmiInterface)
        : mTmiInterface(tmiInterface)
    {...}

    [...]

  private:
    T_TMII& mTmiInterface; // copied by reference
};
```

The internal size of the `SimpleTmi1637Interface` object is just 3 bytes, and
the size of the `SimpleTmi1637FastInterface` is even smaller at 0 bytes, so we
do not save much memory by storing these by reference. But storing the
`mTmiInterface` as a reference causes an unnecessary extra layer of indirection
every time the `mTmiInterface` object is called. In almost every case, I
recommend storing the `XxxInterface` object **by value** into the `MyClass`
object.

<a name="ResourceConsumption"></a>
## Resource Consumption

<a name="FlashAndStaticMemory"></a>
### Flash And Static Memory

The Memory benchmark numbers can be seen in
[examples/MemoryBenchmark](examples/MemoryBenchmark). Here are 2 samples:

**Arduino Nano**

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

**ESP8266**

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

<a name="CpuCycles"></a>
### CPU Cycles

The CPU benchmark numbers can be seen in
[examples/AutoBenchmark](examples/AutoBenchmark). Here are 2 samples:

**Arduino Nano**

```
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   752/  781/  836 |     41.0 |
| SimpleTmi1637FastInterface,1us          |    92/   95/  104 |    336.8 |
| SimpleTmi1638Interface,1us              |   616/  635/  688 |     50.4 |
| SimpleTmi1638FastInterface,1us          |    84/   86/   92 |    372.1 |
+-----------------------------------------+-------------------+----------+
```

**ESP8266**

```
+-----------------------------------------+-------------------+----------+
| Functionality                           |   min/  avg/  max | eff kbps |
|-----------------------------------------+-------------------+----------|
| SimpleTmi1637Interface,1us              |   375/  377/  415 |     84.9 |
| SimpleTmi1638Interface,1us              |   334/  335/  354 |     95.5 |
+-----------------------------------------+-------------------+----------+
```

<a name="SystemRequirements"></a>
## System Requirements

<a name="Hardware"></a>
### Hardware

**Tier 1: Fully Supported**

These boards are tested on each release:

* Arduino Nano (16 MHz ATmega328P)
* SparkFun Pro Micro (16 MHz ATmega32U4)
* STM32 Blue Pill (STM32F103C8, 72 MHz ARM Cortex-M3)
* NodeMCU 1.0 (ESP-12E module, 80MHz ESP8266)
* WeMos D1 Mini (ESP-12E module, 80 MHz ESP8266)
* ESP32 dev board (ESP-WROOM-32 module, 240 MHz dual core Tensilica LX6)
* Teensy 3.2 (72 MHz ARM Cortex-M4)

**Tier 2: Should work**

These boards should work but I don't test them as often:

* ATtiny85 (8 MHz ATtiny85)
* Arduino Pro Mini (16 MHz ATmega328P)
* Mini Mega 2560 (Arduino Mega 2560 compatible, 16 MHz ATmega2560)
* Teensy LC (48 MHz ARM Cortex-M0+)

**Tier 3: May work, but not supported**

* SAMD21 M0 Mini (48 MHz ARM Cortex-M0+)
    * Arduino-branded SAMD21 boards use the ArduinoCore-API, so are explicitly
      blacklisted. See below.
    * Other 3rd party SAMD21 boards *may* work using the SparkFun SAMD core.
    * However, as of SparkFun SAMD Core v1.8.6 and Arduino IDE 1.8.19, I can no
      longer upload binaries to these 3rd party boards due to errors.
    * Therefore, third party SAMD21 boards are now in this new Tier 3 category.
    * This library may work on these boards, but I can no longer support them.

**Tier Blacklisted**

The following boards are *not* supported and are explicitly blacklisted to allow
the compiler to print useful error messages instead of hundreds of lines of
compiler errors:

* Any platform using the ArduinoCore-API
  (https://github.com/arduino/ArduinoCore-api). For example:
    * Nano Every
    * MKRZero
    * Raspberry Pi Pico RP2040

<a name="ToolChain"></a>
### Tool Chain

* [Arduino IDE 1.8.19](https://www.arduino.cc/en/Main/Software)
* [Arduino CLI 0.19.2](https://arduino.github.io/arduino-cli)
* [SpenceKonde ATTinyCore 1.5.2](https://github.com/SpenceKonde/ATTinyCore)
* [Arduino AVR Boards 1.8.4](https://github.com/arduino/ArduinoCore-avr)
* [Arduino SAMD Boards 1.8.9](https://github.com/arduino/ArduinoCore-samd)
* [SparkFun AVR Boards 1.1.13](https://github.com/sparkfun/Arduino_Boards)
* [SparkFun SAMD Boards 1.8.6](https://github.com/sparkfun/Arduino_Boards)
* [STM32duino 2.2.0](https://github.com/stm32duino/Arduino_Core_STM32)
* [ESP8266 Arduino 3.0.2](https://github.com/esp8266/Arduino)
* [ESP32 Arduino 2.0.2](https://github.com/espressif/arduino-esp32)
* [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html)

This library is *not* compatible with:

* Any platform using the
  [ArduinoCore-API](https://github.com/arduino/ArduinoCore-api), for example:
    * [Arduino megaAVR](https://github.com/arduino/ArduinoCore-megaavr/)
    * [MegaCoreX](https://github.com/MCUdude/MegaCoreX)
    * [Arduino SAMD Boards >=1.8.10](https://github.com/arduino/ArduinoCore-samd)

It should work with [PlatformIO](https://platformio.org/) but I have
not tested it.

The library can be compiled under Linux or MacOS (using both g++ and clang++
compilers) using the EpoxyDuino (https://github.com/bxparks/EpoxyDuino)
emulation layer.

<a name="OperatingSystem"></a>
### Operating System

I use Ubuntu 20.04 for the vast majority of my development. I expect that the
library will work fine under MacOS and Windows, but I have not explicitly tested
them.

<a name="License"></a>
## License

[MIT License](https://opensource.org/licenses/MIT)

<a name="FeedbackAndSupport"></a>
## Feedback and Support

If you have any questions, comments, or feature requests for this library,
please use the [GitHub
Discussions](https://github.com/bxparks/AceTMI/discussions) for this project. If
you have bug reports, please file a ticket in [GitHub
Issues](https://github.com/bxparks/AceTMI/issues). Feature requests should go
into Discussions first because they often have alternative solutions which are
useful to remain visible, instead of disappearing from the default view of the
Issue tracker after the ticket is closed.

Please refrain from emailing me directly unless the content is sensitive. The
problem with email is that I cannot reference the email conversation when other
people ask similar questions later.

<a name="Authors"></a>
## Authors

Created by Brian T. Park (brian@xparks.net).

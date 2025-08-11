# Changelog

* Unreleased
* 0.6 (2022-03-01)
    * Add `Tm1638::read()` method to read the keypad data from the TM1638
      controller.
    * Add `Tm1637::read()` method to read the keypad data from the TM1637
      controller.
    * **Potentially Breaking**: Change `SimpleTmi1638Interface::write()` and
      `SimpleTmi1638FastInterface::write()` to return `void` instead of
      `uint8_t` since it was hardcoded to return 0.
* 0.5 (2022-02-02)
    * **Breaking Change**: Rename TM1637 classes for consistency with TM1638
      classes.
        * Rename `SimpleTmiInterface` to `SimpleTmi1637Interface`.
        * Rename `SimpleTmiFastInterface` to `SimpleTmi1637FastInterface`.
* 0.4 (2022-02-01)
    * Add support for the TM1638 controller which uses an SPI-like protocol.
        * Add `SimpleTmi1638Interface` and `SimpleTmi1638FastInterface`.
    * Upgrade tool chain and regenerate `MemoryBenchmark` and `AutoBenchmark`.
        * Arduino IDE from 1.8.13 to 1.8.19
        * Arduino CLI from 0.14.0 to 0.19.2
        * Arduino AVR Core from 1.8.3 to 1.8.4
        * STM32duino from 2.0.0 to 2.2.0
        * ESP8266 Core from 2.7.4 to 3.0.2
        * ESP32 Core from 1.0.6 to 2.0.2
        * Teensyduino from 1.53 to 1.56
* 0.3 (2021-08-17)
    * Rename `sendByte()` to `write()` for consistency with the API of `AceWire`
      and `TwoWire`, since the TM1637 protocol is very similar to I2C.
        * Also avoids confusion with the `send8()` and `send16()` methods of
          `AceSPI` which include the `beginTransaction()` and
          `endTransaction()`.
        * The `write()` methods do *not* include the start and stop conditions.
    * First public release.
* 0.2 (2021-07-30)
    * Add `examples/MemoryBenchmark` and `examples/AutoBenchmark` to calculate
      memory consumption and CPU speed.
    * Invert return value of `writeByte()`, returning 1 on success and 0 on
      failure. Matches the return values of `AceWire` implementations.
    * Add GitHub workflow validation of `examples/*`.
    * Rename `SoftTmi*Interface` to `SimpleTmi*Interface`.
* 0.1 (2021-06-25)
    * First GitHub release.
* 2021-06-24
    * Initial extraction from
      [AceSegment](https://github.com/bxparks/AceSegment) library.

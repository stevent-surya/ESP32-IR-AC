/*
MIT License

Copyright (c) 2022 Brian T. Park

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ACE_TMI_SIMPLE_TMI_1638_FAST_INTERFACE_H
#define ACE_TMI_SIMPLE_TMI_1638_FAST_INTERFACE_H

#include <stdint.h>
#include <Arduino.h> // delayMicroseconds()

namespace ace_tmi {

/**
 * Exactly the same as SimpleTmi1638Interface except that this uses the
 * `digitalWriteFast` library on AVR processors. Normally, the digitalWriteFast
 * library is used to get faster speeds over `digitalWrite()` and `pinMode()`
 * functions.
 *
 * The reason that you may want to use `digitalWriteFast` library is because it
 * consumes far less flash memory than normal `digitalWrite()`. The benchmarks
 * in MemoryBenchmark shows that using this `SimpleTmi1638FastInterface` instead
 * of `SimpleTmi1638Interface` saves ~500 bytes of flash on an AVR processor.
 *
 * Caution: There might be a use-case where the normal `SimpleTmi1638Interface`
 * might consume less flash memory. If your application uses more than one
 * TM1638 LED Module, you will need to create multiple instances of the
 * `Tm1638Module`. But the pin numbers of this class must be a compile-time
 * constants, so different pins means that a different template class is
 * generated. Since the `Tm1638Module` class takes a
 * `SimpleTmi1638FastInterface` as a template argument, each LED Module generate
 * a new template instance of the `Tm1638Module` class.
 *
 * When there are more than some number of TM1638 LED modules, it may actually
 * be more efficient to use the non-fast `SimpleTmi1638Interface`, because you
 * will generate only a single template instantiation. I have not currently done
 * any experiments to see where the break-even point would be.
 *
 * The `delayMicroseconds()` may not be accurate for small values on some
 * processors (e.g. AVR) . The actual minimum value of T_DELAY_MICROS will
 * depend on the capacitance and resistance on the DIO and CLK lines, and the
 * accuracy of the `delayMicroseconds()` function.
 *
 * @tparam T_DIO_PIN pin attached to the DIO data line
 * @tparam T_CLK_PIN pin attached to the CLK clock line
 * @tparam T_STB_PIN pin attached to the STB strobe line
 * @tparam T_DELAY_MICROS delay after each bit transition of DIO or CLK
 */
template <
    uint8_t T_DIO_PIN,
    uint8_t T_CLK_PIN,
    uint8_t T_STB_PIN,
    uint8_t T_DELAY_MICROS
>
class SimpleTmi1638FastInterface {
  public:
    /** Constructor. */
    explicit SimpleTmi1638FastInterface() = default;

    /**
     * Initialize the DIO, CLK, and STB pins.
     *
     * DIO is an open-drain line with pull-up resistors. We must not drive
     * them HIGH actively since that could damage the transitor at the other end
     * of the line pulling LOW. Instead, we go into INPUT mode to let the line
     * to HIGH through the pullup resistor, then go to OUTPUT mode only to pull
     * down.
     */
    void begin() const {
      clockHigh();
      strobeHigh();
      pinModeFast(T_CLK_PIN, OUTPUT);
      pinModeFast(T_STB_PIN, OUTPUT);
      dataHigh(); // open-drain HIGH
    }

    /** Set pins to INPUT mode. */
    void end() const {
      pinModeFast(T_CLK_PIN, INPUT);
      pinModeFast(T_STB_PIN, INPUT);
      dataHigh(); // open-drain HIGH
    }

    /** Generate the SPI-like start condition. */
    void beginTransaction() const {
      clockHigh();
      strobeLow();
      dataHigh(); // open-drain HIGH
    }

    /** Generate the SPI-like stop condition. */
    void endTransaction() const {
      clockHigh();
      strobeHigh();
      dataHigh(); // open-drain HIGH
    }

    /**
     * Send the data byte on the data bus, with LSB first instead of the usual
     * MSB first for SPI.
     *
     * This loop generates slightly asymmetric logic signals because clockLow()
     * lasts for 2*bitDelay(), but clockHigh() lasts for only 1*bitDelay(). This
     * does not seem to cause any problems with the LED modules that I have
     * tested.
     */
    void write(uint8_t data) const {
      for (uint8_t i = 0;  i < 8; ++i) {
        clockLow();
        if (data & 0x1) {
          dataHigh();
        } else {
          dataLow();
        }

        // Device reads DIO on the rising edge of CLK.
        clockHigh();
        // The CLK LOW is twice as long as HIGH. An extra bitDelay() right after
        // the clockHigh() would make the HIGH and LOW states symmetric in
        // duration (if digitalWriteFast() is assumed to be infinitely fast,
        // which it is definitely not). But actual devices that I have tested
        // seem to support the absence of that extra delay. So let's ignore it
        // to make the transfer speed faster.

        data >>= 1;
      }
    }

    /**
     * Read the data byte on the data bus, with LSB first instead of the usual
     * MSB first for SPI.
     *
     * This loop generates slightly asymmetric logic signals because clockLow()
     * lasts for 2*bitDelay(), but clockHigh() lasts for only 1*bitDelay(). This
     * does not seem to cause any problems with the LED modules that I have
     * tested.
     *
     * @return the data byte
     */
    uint8_t read() const {
      // Make sure mDioPin is in INPUT mode because the previous write() may
      // have put mDioPin into open-drain OUTPUT mode.
      dataHigh();

      uint8_t data = 0x0;
      for (uint8_t i = 0;  i < 8; ++i) {
        // Device sets the DIO pin on the falling edge of CLK.
        clockLow();
        uint8_t bit = dataRead();
        clockHigh();
        data >>= 1;
        data |= (bit & 0x1) ? 0x80 : 0x00;
      }
      return data;
    }

    // Use default copy constructor and assignment operator.
    SimpleTmi1638FastInterface(const SimpleTmi1638FastInterface&) = default;
    SimpleTmi1638FastInterface& operator=(const SimpleTmi1638FastInterface&) =
        default;

  private:

    static void bitDelay() { delayMicroseconds(T_DELAY_MICROS); }

    static void clockHigh() { digitalWriteFast(T_CLK_PIN, HIGH); bitDelay(); }

    static void clockLow() { digitalWriteFast(T_CLK_PIN, LOW); bitDelay(); }

    static void dataHigh() { pinModeFast(T_DIO_PIN, INPUT); bitDelay(); }

    static void dataLow() { pinModeFast(T_DIO_PIN, OUTPUT); bitDelay(); }

    static void strobeHigh() { digitalWriteFast(T_STB_PIN, HIGH); bitDelay(); }

    static void strobeLow() { digitalWriteFast(T_STB_PIN, LOW); bitDelay(); }

    static uint8_t dataRead() {
      // Use digitalRead() because digitalReadFast() seems to be buggy for a
      // SparkFun Pro Micro (__AVR_ATmega32U4__).
      uint8_t data = digitalRead(T_DIO_PIN);
      bitDelay();
      return data;
    }

};

} // ace_tmi

#endif

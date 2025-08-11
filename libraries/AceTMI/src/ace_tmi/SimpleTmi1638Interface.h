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

#ifndef ACE_TMI_SIMPLE_TMI_1638_INTERFACE_H
#define ACE_TMI_SIMPLE_TMI_1638_INTERFACE_H

#include <Arduino.h>

namespace ace_tmi {

/**
 * Class that knows how to communicate with a TM1638 chip. It uses a 3-wire
 * (bidirectional DIO, CLK, and STB) protocol that is similar to SPI
 * electrically. the DIO pin is a PMOS open-drain line which means a single
 * transitor on either the master or slave can pull the line LOW, but a pull-up
 * resisotr is required to set the line HIGH. Because it is an open-drain, we
 * must make sure that the microcontroller does *not* actively drive this line
 * HIGH, otherwise, the output pin of the MCU at 5V (HIGH) becomes directly
 * connected to the 0V (LOW) of the transistor on the device pulling it LOW,
 * with no current limiting resistor. Either of MCU or the device can become
 * damaged. To set the line HIGH or LOW, we set the output level to LOW, then
 * use the pinMode() function to either INPUT (to get a HIGH value) or OUTPUT
 * (to get a LOW value).
 *
 * The logical protocol of the TM1638 is similar to SPI in the following ways:
 *
 *    * The falling edge of STB from the MCU signals the start of command byte.
 *    * The DIO line sends from the MCU to the chip on the rising edge of the
 *      CLK.
 *
 * The difference is:
 *
 *    * The first byte sent to the TM1638 is a command byte.
 *    * The bytes are sent LSB first instead of the usual MSB first on SPI.
 *
 * Since the protocol does not match SPI, we cannot use the hardware SPI
 * capabilities of the microcontroller, so we have to implement a software
 * version of this protocol.
 */
class SimpleTmi1638Interface {
  public:
    /**
     * Constructor.
     *
     * The `delayMicroseconds()` may not be accurate for small values on some
     * processors (e.g. AVR) . The actual minimum value of delayMicros will
     * depend on the capacitance and resistance on the DIO and CLK lines, and
     * the accuracy of the `delayMicroseconds()` function.
     *
     * @param dioPin pin attached to the DIO data line
     * @param clkPin pin attached to the CLK clock line
     * @param stbPin pin attached to the STB strobe line
     * @param delayMicros delay after each bit transition of CLK
     */
    explicit SimpleTmi1638Interface(
        uint8_t dioPin,
        uint8_t clkPin,
        uint8_t stbPin,
        uint8_t delayMicros
    ) :
        mDioPin(dioPin),
        mClkPin(clkPin),
        mStbPin(stbPin),
        mDelayMicros(delayMicros)
    {}

    /**
     * Initialize the DIO, CLK and STB pins.
     *
     * DIO is an open-drain line with pull-up resistors. We must not drive it
     * HIGH actively since that could damage the transitor at the other end of
     * the line pulling LOW. Instead, we go into INPUT mode to let the line to
     * HIGH through the pullup resistor, then go to OUTPUT mode only to pull
     * down.
     */
    void begin() const {
      clockHigh();
      strobeHigh();
      pinMode(mClkPin, OUTPUT);
      pinMode(mStbPin, OUTPUT);
      dataHigh(); // open-drain HIGH
    }

    /** Set pins to INPUT mode. */
    void end() const {
      pinMode(mClkPin, INPUT);
      pinMode(mStbPin, INPUT);
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
        // the clockHigh() would make the HIGH and LOW symmetric in duration (if
        // digitalWrite() is assumed to be infinitely fast, which it is
        // definitely not). But actual devices that I have tested seem to
        // support the absence of that extra delay. So let's ignore it to make
        // the transfer speed faster.

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
    SimpleTmi1638Interface(const SimpleTmi1638Interface&) = default;
    SimpleTmi1638Interface& operator=(const SimpleTmi1638Interface&) = default;

  private:
    void bitDelay() const { delayMicroseconds(mDelayMicros); }

    void clockHigh() const { digitalWrite(mClkPin, HIGH); bitDelay(); }

    void clockLow() const { digitalWrite(mClkPin, LOW); bitDelay(); }

    void dataHigh() const { pinMode(mDioPin, INPUT); bitDelay(); }

    void dataLow() const { pinMode(mDioPin, OUTPUT); bitDelay(); }

    void strobeHigh() const { digitalWrite(mStbPin, HIGH); bitDelay(); }

    void strobeLow() const { digitalWrite(mStbPin, LOW); bitDelay(); }

    uint8_t dataRead() const {
      uint8_t data = digitalRead(mDioPin);
      bitDelay();
      return data;
    }

  private:
    uint8_t const mDioPin;
    uint8_t const mClkPin;
    uint8_t const mStbPin;
    uint8_t const mDelayMicros;
};

} // ace_tmi

#endif

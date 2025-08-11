/*
MIT License

Copyright (c) 2021 Brian T. Park

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

#ifndef ACE_TMI_SIMPLE_TMI_1637_INTERFACE_H
#define ACE_TMI_SIMPLE_TMI_1637_INTERFACE_H

#include <Arduino.h>

namespace ace_tmi {

/**
 * Class that knows how to communicate with a TM1637 chip. It uses a 2-wire
 * (Clock and a bidirectional DIO) protocol that is similar to I2C electrically.
 * Both the Clock and Data pins are open-drain which means a single transitor on
 * either the master or slave can pull the line LOW, but a pull-up resisotr is
 * required to set the line HIGH. Because these are open-drain, we must make
 * sure that the microcontroller does *not* actively drive these lines HIGH,
 * otherwise, the output pin of the MCU at 5V (HIGH) becomes directly connected
 * to the 0V (LOW) of the transistor on the device pulling it LOW, with no
 * current limiting resistor. Either of MCU or the device can become damaged. To
 * set the line HIGH or LOW, we set the output level to LOW, then use the
 * pinMode() function to either INPUT (to get a HIGH value) or OUTPUT (to get a
 * LOW value).
 *
 * The logical protocol of the TM1637 is similar to I2C in the following ways:
 *
 *    * The start and stop conditions are the same.
 *    * Data transfer happens on the rising edge of the CLK signal.
 *    * The slave sends back a one-bit ACK/NACK after the 8th bit of the CLK.
 *
 * The difference is:
 *
 *    * There is no I2C address byte, so only a single TM1637 device can be on
 *      the bus.
 *    * The first byte sent to the TM1637 is a command byte.
 *    * The bytes are sent LSB first instead of the usual MSB first on I2C.
 *
 * Since the protocol does not match I2C, we cannot use the hardware I2C
 * capabilities of the microcontroller, so we have to implement a software
 * version of this protocol.
 */
class SimpleTmi1637Interface {
  public:
    /**
     * Constructor.
     *
     * The `delayMicroseconds()` may not be accurate for small values on some
     * processors (e.g. AVR) . The actual minimum value of delayMicro will
     * depend on the capacitance and resistance on the DIO and CLK lines, and
     * the accuracy of the `delayMicroseconds()` function.
     *
     * @param dioPin pin attached to the data line
     * @param clkPin pin attached to the clock line
     * @param delayMicros delay after each bit transition of DIO or CLK
     */
    explicit SimpleTmi1637Interface(
        uint8_t dioPin,
        uint8_t clkPin,
        uint8_t delayMicros
    ) :
        mDioPin(dioPin),
        mClkPin(clkPin),
        mDelayMicros(delayMicros)
    {}

    /**
     * Initialize the DIO and CLK pins.
     *
     * These are open-drain lines, with pull-up resistors. We must not drive
     * them HIGH actively since that could damage the transitor at the other
     * end of the line pulling LOW. Instead, we go into INPUT mode to let the
     * line to HIGH through the pullup resistor, then go to OUTPUT mode only
     * to pull down.
     */
    void begin() const {
      digitalWrite(mClkPin, LOW);
      digitalWrite(mDioPin, LOW);

      // Begin with both lines at HIGH.
      clockHigh();
      dataHigh();
    }

    /** Set dio and clk pins to INPUT mode. */
    void end() const {
      clockHigh();
      dataHigh();
    }

    /** Generate the I2C-like start condition. */
    void startCondition() const {
      clockHigh();
      dataHigh();

      dataLow();
      clockLow();
    }

    /** Generate the I2C-like stop condition. */
    void stopCondition() const {
      // clock will always be LOW when this is called
      dataLow();
      clockHigh();
      dataHigh();
    }

    /**
     * Send the data byte on the data bus, with LSB first instead of the usual
     * MSB first for I2C.
     *
     * This loop generates slightly asymmetric logic signals because clockLow()
     * lasts for 2*bitDelay(), but clockHigh() lasts for only 1*bitDelay(). This
     * does not seem to cause any problems with the LED modules that I have
     * tested.
     *
     * @return 1 if device responded with ACK, 0 for NACK. (This retains
     *    consistency with AceWire's I2C `write()` methods which return the
     *    number of bytes transfered.)
     */
    uint8_t write(uint8_t data) const {
      for (uint8_t i = 0;  i < 8; ++i) {
        if (data & 0x1) {
          dataHigh();
        } else {
          dataLow();
        }

        // Device reads DIO on the rising edge of CLK.
        clockHigh();

        // An extra bitDelay() here would make the HIGH and LOW states symmetric
        // in duration (if digitalWrite() is assumed to be infinitely fast,
        // which it is definitely not). But actual devices that I have tested
        // seem to support the absence of that extra delay. So let's ignore it
        // to make the transfer speed faster.
        clockLow();
        data >>= 1;
      }

      uint8_t ack = readAck();
      return ack ^ 0x1; // invert the 0 and 1
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

      // Read the ACK from device
      uint8_t ack = readAck();
      (void) ack;

      return data;
    }

    // Use default copy constructor and assignment operator.
    SimpleTmi1637Interface(const SimpleTmi1637Interface&) = default;
    SimpleTmi1637Interface& operator=(const SimpleTmi1637Interface&) = default;

  private:
    /**
     * Read the ACK/NACK bit from the device after the falling edge of the 8th
     * CLK, which happens in the write() loop above.
     *
     * @return 0 for ACK (active LOW), 1 or NACK (passive HIGH).
     */
    uint8_t readAck() const {
      // Go into INPUT mode, reusing dataHigh(), saving 10 flash bytes on AVR.
      dataHigh();

      // DIO is supposed to remain stable after CLK is set HIGH.
      clockHigh();

      uint8_t ack = digitalRead(mDioPin);

      // Device releases DIO upon falling edge of the 9th CLK.
      clockLow();
      return ack;
    }

    void bitDelay() const { delayMicroseconds(mDelayMicros); }

    void clockHigh() const { pinMode(mClkPin, INPUT); bitDelay(); }

    void clockLow() const { pinMode(mClkPin, OUTPUT); bitDelay(); }

    void dataHigh() const { pinMode(mDioPin, INPUT); bitDelay(); }

    void dataLow() const { pinMode(mDioPin, OUTPUT); bitDelay(); }

    uint8_t dataRead() const {
      uint8_t data = digitalRead(mDioPin);
      bitDelay();
      return data;
    }

  private:
    uint8_t const mDioPin;
    uint8_t const mClkPin;
    uint8_t const mDelayMicros;
};

} // ace_tmi

#endif

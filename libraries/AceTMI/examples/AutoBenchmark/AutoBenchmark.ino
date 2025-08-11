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

/*
 * A sketch that generates the min/avg/max duration (in microsecondes) of
 * various implementations in the AceTMI library. See the generated README.md
 * for more information.
 */

#include <Arduino.h>
#include <AceCommon.h> // TimingStats
#include <AceTMI.h>

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
#include <digitalWriteFast.h>
#include <ace_tmi/SimpleTmi1637FastInterface.h>
#include <ace_tmi/SimpleTmi1638FastInterface.h>
#endif

using namespace ace_tmi;
using ace_common::TimingStats;

#if ! defined(SERIAL_PORT_MONITOR)
#define SERIAL_PORT_MONITOR Serial
#endif

//-----------------------------------------------------------------------------
// Setup parameters for a TM1637 controller chip. An actual device does NOT need
// to be on the bus (because ACK/NACK are ignored during the runs). We reuse the
// built-in definitions of the I2C SDA and SCL pins, because those macros are
// guaranteed to be defined on all (?) platforms, and it is possible to attach a
// TM1637 to those GPIO pins if I2C is not used for anything else.
//-----------------------------------------------------------------------------

const uint8_t BIT_DELAY = 1;
const uint8_t CLK_PIN = SCL;
const uint8_t DIO_PIN = SDA;
const uint8_t STB_PIN = 10;

//-----------------------------------------------------------------------------
// Timing stats.
//-----------------------------------------------------------------------------

/** Print the result for each LedMatrix algorithm. */
static void printStats(
    const __FlashStringHelper* name,
    const TimingStats& stats,
    uint16_t numSamples) {
  SERIAL_PORT_MONITOR.print(name);
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMin());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getAvg());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.print(stats.getMax());
  SERIAL_PORT_MONITOR.print(' ');
  SERIAL_PORT_MONITOR.println(numSamples);
}

TimingStats timingStats;

//-----------------------------------------------------------------------------
// Benchmark various TMI implementations.
//-----------------------------------------------------------------------------

template <typename T_TMII>
void runBenchmark(
    const __FlashStringHelper* name,
    T_TMII& tmiInterface) {

  const uint16_t numSamples = 20;

  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    tmiInterface.startCondition();
    // Send 4 bytes, approximating a 4-digit TM1637 LED module.
    tmiInterface.write(0x11);
    tmiInterface.write(0x22);
    tmiInterface.write(0x33);
    tmiInterface.write(0x44);
    tmiInterface.stopCondition();
    uint16_t endMicros = micros();

    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

template <typename T_TMII>
void runBenchmark1638(
    const __FlashStringHelper* name,
    T_TMII& tmiInterface) {

  const uint16_t numSamples = 20;

  timingStats.reset();
  for (uint16_t i = 0; i < numSamples; ++i) {
    uint16_t startMicros = micros();
    tmiInterface.beginTransaction();
    // Send 4 bytes, approximating a 4-digit TM1637 LED module.
    tmiInterface.write(0x11);
    tmiInterface.write(0x22);
    tmiInterface.write(0x33);
    tmiInterface.write(0x44);
    tmiInterface.endTransaction();
    uint16_t endMicros = micros();

    timingStats.update(endMicros - startMicros);
    yield();
  }

  printStats(name, timingStats, numSamples);
}

void runSimpleTmi() {
  using TmiInterface = SimpleTmi1637Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, BIT_DELAY);
  tmiInterface.begin();
  runBenchmark(F("SimpleTmi1637Interface,1us"), tmiInterface);
  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runSimpleTmiFast() {
  using TmiInterface = SimpleTmi1637FastInterface<DIO_PIN, CLK_PIN, BIT_DELAY>;
  TmiInterface tmiInterface;
  tmiInterface.begin();
  runBenchmark(F("SimpleTmi1637FastInterface,1us"), tmiInterface);
  tmiInterface.end();
}
#endif

void runSimpleTmi1638() {
  using TmiInterface = SimpleTmi1638Interface;
  TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY);
  tmiInterface.begin();
  runBenchmark1638(F("SimpleTmi1638Interface,1us"), tmiInterface);
  tmiInterface.end();
}

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
void runSimpleTmi1638Fast() {
  using TmiInterface = SimpleTmi1638FastInterface<
      DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY>;
  TmiInterface tmiInterface;
  tmiInterface.begin();
  runBenchmark1638(F("SimpleTmi1638FastInterface,1us"), tmiInterface);
  tmiInterface.end();
}
#endif

//-----------------------------------------------------------------------------
// runBenchmarks()
//-----------------------------------------------------------------------------

void runBenchmarks() {
  runSimpleTmi();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runSimpleTmiFast();
#endif

  runSimpleTmi1638();
#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  runSimpleTmi1638Fast();
#endif
}

//-----------------------------------------------------------------------------
// sizeof()
//-----------------------------------------------------------------------------

void printSizeOf() {
  SERIAL_PORT_MONITOR.print(F("sizeof(SimpleTmi1637Interface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleTmi1637Interface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(
      F("sizeof(SimpleTmi1637FastInterface<4, 5, 100>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleTmi1637FastInterface<4, 5, 100>));
#endif

  SERIAL_PORT_MONITOR.print(F("sizeof(SimpleTmi1638Interface): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleTmi1638Interface));

#if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
  SERIAL_PORT_MONITOR.print(
      F("sizeof(SimpleTmi1638FastInterface<4, 5, 6, 1>): "));
  SERIAL_PORT_MONITOR.println(sizeof(SimpleTmi1638FastInterface<4, 5, 6, 1>));
#endif
}

//-----------------------------------------------------------------------------

void setup() {
#if ! defined(EPOXY_DUINO)
  delay(1000); // Wait for stability on some boards, otherwise garage on Serial
#endif

  SERIAL_PORT_MONITOR.begin(115200);
  while (!SERIAL_PORT_MONITOR); // Wait for Leonardo/Micro

  SERIAL_PORT_MONITOR.println(F("SIZEOF"));
  printSizeOf();

  SERIAL_PORT_MONITOR.println(F("BENCHMARKS"));
  runBenchmarks();

  SERIAL_PORT_MONITOR.println(F("END"));

#if defined(EPOXY_DUINO)
  exit(0);
#endif
}

void loop() {}

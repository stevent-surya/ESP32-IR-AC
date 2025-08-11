/*
 * A program which compiles different implementations of the TM1637 and TM1638
 * communcation protocol in the AceTMI library to determine the flash and static
 * memory sizes from the output of the compiler. See the generated README.md for
 * details.
 */

#include <Arduino.h>

// DO NOT MODIFY THIS LINE. This will be overwritten by collect.sh on each
// iteration, incrementing from 0 to N. The Arduino IDE will compile the
// program, then the script will extract the flash and static memory usage
// numbers printed out by the Arduino compiler. The numbers will be printed on
// the STDOUT, which then can be saved to a file specific for a particular
// hardware platform, e.g. "nano.txt" or "esp8266.txt".
#define FEATURE 0

// List of features of AceTMI that we want to gather memory usage numbers.
#define FEATURE_BASELINE 0
#define FEATURE_TMI 1
#define FEATURE_TMI_FAST 2
#define FEATURE_TMI_1638 3
#define FEATURE_TMI_1638_FAST 4

// A volatile integer to prevent the compiler from optimizing away the entire
// program.
volatile int disableCompilerOptimization = 0;

#if FEATURE > FEATURE_BASELINE
  #include <AceTMI.h>
  #if defined(ARDUINO_ARCH_AVR) || defined(EPOXY_DUINO)
    #include <digitalWriteFast.h>
    #include <ace_tmi/SimpleTmi1637FastInterface.h>
    #include <ace_tmi/SimpleTmi1638FastInterface.h>
  #endif
  using namespace ace_tmi;

  const uint8_t CLK_PIN = 16;
  const uint8_t DIO_PIN = 10;
  const uint8_t STB_PIN = 11;
  const uint8_t BIT_DELAY = 100;

  #if FEATURE == FEATURE_TMI
    using TmiInterface = SimpleTmi1637Interface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, BIT_DELAY);

  #elif FEATURE == FEATURE_TMI_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using TmiInterface = SimpleTmi1637FastInterface<
        DIO_PIN, CLK_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;

  #elif FEATURE == FEATURE_TMI_1638
    using TmiInterface = SimpleTmi1638Interface;
    TmiInterface tmiInterface(DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY);

  #elif FEATURE == FEATURE_TMI_1638_FAST
    #if ! defined(ARDUINO_ARCH_AVR) && ! defined(EPOXY_DUINO)
      #error Unsupported FEATURE on this platform
    #endif

    using TmiInterface = SimpleTmi1638FastInterface<
        DIO_PIN, CLK_PIN, STB_PIN, BIT_DELAY>;
    TmiInterface tmiInterface;
  #endif
#endif

// TeensyDuino seems to pull in malloc() and free() when a class with virtual
// functions is used polymorphically. This causes the memory consumption of
// FEATURE_BASELINE (which normally has no classes defined, so does not include
// malloc() and free()) to be artificially small which throws off the memory
// consumption calculations for all subsequent features. Let's define a
// throw-away class and call its method for all FEATURES, including BASELINE.
#if defined(TEENSYDUINO)
  class FooClass {
    public:
      virtual void doit() {
        disableCompilerOptimization = 0;
      }
  };

  FooClass* foo;
#endif

void setup() {
  #if defined(TEENSYDUINO)
    foo = new FooClass();
  #endif

  disableCompilerOptimization = 3;

// In the following, I used to grab the output of patterns[] and write to
// disableCompilerOptimization to prevent the compiler from optimizing away the
// entire program. But after templatizing ScanningModule, pattterns variable is
// no longer accessible. But it does not matter because I realized that
// ScanningModule performs a digitalWrite(), which has the same effect of
// disabling optimizations.

  #if FEATURE == FEATURE_BASELINE
    // do nothing
  #elif FEATURE == FEATURE_TMI \
      || FEATURE == FEATURE_TMI_FAST \
      || FEATURE == FEATURE_TMI_1638 \
      || FEATURE == FEATURE_TMI_1638_FAST
    tmiInterface.begin();
  #endif
}

void loop() {
  #if defined(TEENSYDUINO)
    foo->doit();
  #endif

  #if FEATURE == FEATURE_BASELINE
    // do nothing
  #elif FEATURE == FEATURE_TMI \
      || FEATURE == FEATURE_TMI_FAST
    tmiInterface.startCondition();
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.stopCondition();
  #elif FEATURE == FEATURE_TMI_1638 \
      || FEATURE == FEATURE_TMI_1638_FAST
    tmiInterface.beginTransaction();
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.write(0x11);
    tmiInterface.endTransaction();
  #else
    #error Unknown FEATURE
  #endif
}

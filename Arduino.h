#ifndef _ARDUINO_H_
#define _ARDUINO_H_

#include <stdint.h>
#include <stdbool.h>

// Arduino-style types
typedef bool boolean;
typedef uint8_t byte;

// Logic levels
#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW  0x0
#endif

// Pin modes (not actually used in your Si5351 lib)
#ifndef INPUT
#define INPUT 0x0
#endif
#ifndef OUTPUT
#define OUTPUT 0x1
#endif

// Time functions (stubs — add Pico SDK equivalents if needed later)
inline unsigned long millis() { return 0; }
inline void delay(unsigned long) {}

// Serial placeholder (only if some library tries to use Serial.print)
struct DummySerial {
    template <typename T>
    void print(T) {}
    template <typename T>
    void println(T) {}
};
static DummySerial Serial;

#endif // _ARDUINO_H_

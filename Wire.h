#ifndef _WIRE_H_
#define _WIRE_H_

#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include <cstdint>

class WireClass {
public:
    WireClass(i2c_inst_t *i2cPort = i2c0, uint sdaPin = 16, uint sclPin = 17);

    void begin(); // init with defaults
    void beginTransmission(uint8_t address);
    uint8_t endTransmission();

    void write(uint8_t data);
    void write(uint8_t *data, size_t length);

    uint8_t requestFrom(uint8_t address, uint8_t quantity);
    int available();
    int read();

private:
    i2c_inst_t *i2cPort;
    uint sdaPin;
    uint sclPin;
    uint8_t txAddress;
    static const int MAX_BUFFER = 32;

    uint8_t txBuffer[MAX_BUFFER];
    int txIndex;
    int txLength;

    uint8_t rxBuffer[MAX_BUFFER];
    int rxIndex;
    int rxLength;
};

extern WireClass Wire;

#endif

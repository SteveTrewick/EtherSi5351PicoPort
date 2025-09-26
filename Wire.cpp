#include "Wire.h"

WireClass::WireClass(i2c_inst_t *i2cPort, uint sdaPin, uint sclPin)
    : i2cPort(i2cPort), sdaPin(sdaPin), sclPin(sclPin) {
    txIndex = 0;
    txLength = 0;
    rxIndex = 0;
    rxLength = 0;
    txAddress = 0;
}

void WireClass::begin() {
    // Initialize I2C at 100kHz
    i2c_init(i2cPort, 100 * 1000);

    gpio_set_function(sdaPin, GPIO_FUNC_I2C);
    gpio_set_function(sclPin, GPIO_FUNC_I2C);

    gpio_pull_up(sdaPin);
    gpio_pull_up(sclPin);
}

void WireClass::beginTransmission(uint8_t address) {
    txAddress = address;
    txIndex = 0;
    txLength = 0;
}

uint8_t WireClass::endTransmission() {
    int ret = i2c_write_blocking(i2cPort, txAddress, txBuffer, txLength, false);
    return (ret < 0) ? 4 : 0; // 0 = success, 4 = error (like Arduino Wire)
}

void WireClass::write(uint8_t data) {
    if (txLength < MAX_BUFFER) {
        txBuffer[txLength++] = data;
    }
}

void WireClass::write(uint8_t *data, size_t length) {
    for (size_t i = 0; i < length && txLength < MAX_BUFFER; i++) {
        txBuffer[txLength++] = data[i];
    }
}

uint8_t WireClass::requestFrom(uint8_t address, uint8_t quantity) {
    if (quantity > MAX_BUFFER) quantity = MAX_BUFFER;
    int ret = i2c_read_blocking(i2cPort, address, rxBuffer, quantity, false);
    if (ret < 0) {
        rxLength = 0;
        return 0;
    }
    rxLength = ret;
    rxIndex = 0;
    return rxLength;
}

int WireClass::available() {
    return rxLength - rxIndex;
}

int WireClass::read() {
    if (rxIndex < rxLength) {
        return rxBuffer[rxIndex++];
    }
    return -1;
}

WireClass Wire; // global instance

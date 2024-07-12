#ifndef UARTHANDLER_H
#define UARTHANDLER_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

class UARThandler {
public:
    UARThandler(uint txPin, uint rxPin, uint baudRate);

    void init();
    void sendChar(char c);
    void sendString(const char* s);
    char receiveChar();

private:
    uint txPin;
    uint rxPin;
    uint baudRate;
    PIO pio;
    uint smTx;
    uint smRx;
    uint offsetTx;
    uint offsetRx;
};

#endif // UARTHANDLER_H

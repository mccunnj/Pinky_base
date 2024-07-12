#include "UARThandler.h"
#include "uart_tx.pio.h"
#include "uart_rx.pio.h"

UARThandler::UARThandler(uint txPin, uint rxPin, uint baudRate)
    : txPin(txPin), rxPin(rxPin), baudRate(baudRate), pio(pio1), smTx(0), smRx(1), offsetTx(0), offsetRx(0) {}

void UARThandler::init() {
    // Load the TX program into PIO1
    offsetTx = pio_add_program(pio, &uart_tx_program);
    uart_tx_program_init(pio, smTx, offsetTx, txPin, baudRate);

    // Load the RX program into PIO1
    offsetRx = pio_add_program(pio, &uart_rx_program);
    uart_rx_program_init(pio, smRx, offsetRx, rxPin, baudRate);
}

void UARThandler::sendChar(char c) {
    pio_sm_put_blocking(pio, smTx, (uint32_t)c);
}

void UARThandler::sendString(const char* s) {
    while (*s) {
        sendChar(*s++);
    }
}

char UARThandler::receiveChar() {
    io_rw_8 *rxfifo_shift = (io_rw_8*)&pio->rxf[smRx] + 3;
    while (pio_sm_is_rx_fifo_empty(pio, smRx)) {
        tight_loop_contents();
    }
    return (char)*rxfifo_shift;
}

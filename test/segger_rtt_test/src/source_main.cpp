// see https://www.segger.com/products/debug-probes/j-link/technology/real-time-transfer/about-real-time-transfer/


#include <mbed_events.h>
#include <mbed.h>

#include "SEGGER_RTT.h"

DigitalOut  led1(LED1, 1);

static EventQueue eventQueue(10 * EVENTS_EVENT_SIZE);


int main(void) {
    auto f = []()->void { led1 = !led1; };
    eventQueue.call_every(500, f);

    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    SEGGER_RTT_WriteString(0, "SEGGER Real-Time-Terminal Sample\r\n\r\n");
    SEGGER_RTT_WriteString(0, "###### Testing SEGGER_printf() ######\r\n");

    eventQueue.dispatch_forever();


    return 0;
}
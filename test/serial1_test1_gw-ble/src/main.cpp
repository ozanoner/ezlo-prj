/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


 // TRY THIS: https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-ble-LEDBlinker/file/bf9a45219fe2/source/main.cpp/


// // USB Pins
    // USBTX = P0_10,
    // USBRX = P0_11,

#include <mbed_events.h>
#include <mbed.h>
#include "PinNames.h"



#include "SEGGER_RTT.h"
#define DPRN(...) SEGGER_RTT_printf(0, __VA_ARGS__)


static EventQueue eventQueue;
Serial esp32_comm(P0_6, P0_8, 9600);

void serialRead() {
    while(esp32_comm.readable())
        DPRN("%c",(char)esp32_comm.getc());
}

int main()
{
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    DPRN("started\n");
    wait_ms(100);

    // esp32_comm.attach(serialRead);
   

    eventQueue.call_every(100, serialRead);

    eventQueue.dispatch_forever();
    return 0;
}


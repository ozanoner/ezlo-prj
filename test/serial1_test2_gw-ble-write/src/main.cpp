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
Serial pc(USBTX, USBRX, 9600);

DigitalOut  led1(LED1, 1);
DigitalOut  led_err(LED2, 0);


int i=0;


int main()
{
    eventQueue.call_every(100, []()->void{
        led1 = !led1;
    });  
    // SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    // DPRN("started\n");
    pc.set_flow_control(mbed::SerialBase::Flow::Disabled, NC, NC);
    pc.format(8, SerialBase::None, 1);
    pc.set_blocking(false);
    // 6->tx , 8->rx
    // pc.attach([]()->void{
    //     while(pc.readable())
    //         DPRN("%c",(char)pc.getc());
    // });

    eventQueue.call_every(1000, []()->void{
        if(pc.printf("aa")==2) {
            led_err = 1;
        }
        else {
            led_err = 0;
        }
    });
   
    // DPRN("after attach\n");

    eventQueue.dispatch_forever();
    return 0;
}



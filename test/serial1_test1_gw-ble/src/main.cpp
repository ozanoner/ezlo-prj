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
// Serial pc(USBTX, USBRX, 9600);

DigitalOut  led1(LED1, 1);


int i=0;

/*
int main()
{
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    DPRN("started\n");

    
    wait_ms(1000);

    Serial esp32_comm(P0_26, P0_27, 9600);
    esp32_comm.format(8, SerialBase::None, 1);
    esp32_comm.set_blocking(false);
    // 6->tx , 8->rx
    esp32_comm.attach([&s=esp32_comm]()->void{
        while(s.readable())
            DPRN("%c",(char)s.getc());
    });
   
    DPRN("after attach\n");

    // pc.attach(serialRead_pc);


    while(true) {
        DPRN("in-while\n");
        wait_ms(500);
        led1 = !led1;
        esp32_comm.printf("nrf %d\n", i++);
    }

    // eventQueue.call_every(500, [&s=esp32_comm]()->void {
    //     led1 = !led1;
    //     if(s.writeable()) {
    //         s.printf("nrf %d\n", i++);
    //         fflush(s);
    //     }
    //     else
    //         DPRN("serial not writeable\n");
    // });

    // eventQueue.call_every(500, []()->void{
    //     DPRN("jlink rtt debug\n");
    // });

    eventQueue.dispatch_forever();
    return 0;
}

*/

#include <UARTSerial.h>
#include <string.h>

/*
int main()
{
    // SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    // DPRN("started\n");

    
    wait_ms(1000);


    // UARTSerial esp32_comm(P0_26, P0_27, 9600);
    // esp32_comm.set_blocking(false);

    char buff[64];

    // esp32_comm.format(8, SerialBase::None, 1);
    // esp32_comm.attach([&s=esp32_comm]()->void{
    //     while(s.readable())
    //         DPRN("%c",(char)s.getc());
    // });

    // esp32_comm.sigio([&s=esp32_comm]()->void{
    //     char buff[64];
    //     if(s.readable()) {
    //         int i=s.read(buff, 64);
    //         buff[i]=0;
    //         DPRN(buff);
    //     }
    // });

    // while(true) {
    //     DPRN("in-while\n");
    //     wait_ms(500);
    //     led1 = !led1;
    //     sprintf(buff,"aaa");
    //     if(esp32_comm.write(buff, 3)<0) {
    //         DPRN("write error\n");
    //     }
    // }

    eventQueue.dispatch_forever();
    return 0;
}
*/


/*
int main() {
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    // Serial pc(P0_26, P0_27, 9600);
    Serial pc(USBTX, USBRX, 9600);
    pc.set_blocking(false);

    

    pc.attach([&pc]()->void {
        while(pc.readable()) {
            DPRN("r(%c)", (char)pc.getc());
        }
    });

    eventQueue.call_every(1000, [&pc]()->void {
        led1 = !led1;
    
        char buff[64] = "aaa";
        DPRN("writing data\n");
        
        if(int i=pc.printf(buff)) {
            if(i!=3)
                DPRN("write err %d\n", i);
        }
    });
    

    eventQueue.dispatch_forever();
    return 0;
}
*/

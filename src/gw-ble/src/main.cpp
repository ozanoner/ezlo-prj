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
#include "EspConn.h"
#include "SEGGER_RTT.h"




static EventQueue eventQueue;
Serial pc(P0_6, P0_8, 9600);

// InterruptIn btn1(BUTTON1);

// DigitalOut  led1(LED1, 1);
// DigitalOut  led_err(LED2, 0);
// DigitalOut  led_btn1(LED3, 1);


DigitalOut statusLed(P0_9, 1);
// DigitalOut statusLed(LED1, 1);

int i=0;

EspConn espConn(pc, eventQueue);

void espDataReceivedCb(std::shared_ptr<const char*> data) {
// void espDataReceivedCb(const char* data) {
    // DPRN(*data);
    // espConn.send("[info] received: %s", *data);

    DPRN("[info] in callback");
    statusLed = !statusLed;
}

int main()
{
    DPRN("[info] started");
    espConn.init(espDataReceivedCb);
    eventQueue.call_every(1000, []()->void {
        // statusLed = !statusLed;
        // pc.printf("%d\n", i++);
        espConn.send("%d", i++);
    });  
    // pc.set_flow_control(mbed::SerialBase::Flow::Disabled, NC, NC);
    // pc.format(8, SerialBase::None, 1);
    // pc.set_blocking(false);
    // // 6->tx , 8->rx
    // pc.attach([]()->void{
    //     char c;
    //     while(pc.readable()) {
    //         c = pc.getc();
    //         if(c=='\n')
    //             DPRN("\n");
    //         else
    //             DPRN("%c",c);
    //     }
    // });

    // btn1.rise(btn1Pressed);
    // btn1.fall(btn1Released);



    eventQueue.dispatch_forever();
    return 0;
}



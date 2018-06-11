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
#include <string>
#include <sstream>
#include <cstdio>

#include "HAHardwareDefs.h"
#include "EspConn.h"
#include "BleConn.h"

#include "SEGGER_RTT.h"

// #define BLE_DEBUG_PRINT

static EventQueue eventQueue;
Serial pc(UART2_TX, UART2_RX, 9600);
EspConn espConn(pc, eventQueue);
BleConn bleConn(eventQueue);

DigitalOut statusLed(STATUS_LED, 1);


void espDataReceivedCb(const char* data) {
    bleConn.userCommand(data);
}

void bleDebugPrint(const char* fmt, va_list arg) {
    int buffs = vsnprintf(0,0,fmt, arg);
    if(buffs<0) {
        DPRN(fmt, arg);
        DPRN("[error] bleDebugPrint failed\n");
        return;
    }
    ++buffs;
    char buff[buffs];
    vsnprintf(buff, buffs, fmt, arg);
    if(buffs>128)
        buff[128]=0; // limit to 128

    for(int i=0; i<buffs && buff[i]; i++) {
        if(buff[i] == '"')
            buff[i] = ' ';
    }
    DPRN(buff);
    
    const char* newFmt = "{\"ble_debug\":\"%s\"}";    
    espConn.send(newFmt, buff);
}

void bleResponseCallback(const char* resp) {
    espConn.send(resp);
}

void toggleStatusLed() {
    statusLed = !statusLed;
}

int main()
{
    espConn.init(espDataReceivedCb);
    
    eventQueue.call_in(30000, []()->void {
        bleConn.init(bleResponseCallback, bleDebugPrint);
    });
    eventQueue.call_every(1000, toggleStatusLed);
    eventQueue.dispatch_forever();

    return 0;
}



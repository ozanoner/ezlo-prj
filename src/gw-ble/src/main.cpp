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

#include "HAHardwareDefs.h"


#include <mbed_events.h>
#include <mbed.h>
#include "PinNames.h"
#include "EspConn.h"
#include "BleConn.h"

#include "mbed_stats.h"


#include "SEGGER_RTT.h"
// #include "json/src/json.hpp"
// using json = nlohmann::json;



static EventQueue eventQueue;
Serial pc(UART2_TX, UART2_RX, 9600);
EspConn espConn(pc, eventQueue);
BleConn bleConn(eventQueue);

DigitalOut statusLed(STATUS_LED, 1);
// Ticker statusToggler;


void espDataReceivedCb(const char* data) {
    // DPRN("[info] in callback");
    // statusLed = !statusLed;
    
    bleConn.userCommand(data);

    // auto root = json::parse(*data);

    // int devId = root["dev"]; // specific device id
    // int stateId = root["state"]; // state enum, ON_OFF, DIMMER etc
    // int cmd = root["cmd"]; // get=0 | set=1
    // int setValue = root["val"]; // state specific value if cmd=1
}

void bleDebugPrint(const char* fmt, va_list arg) {
    espConn.send(fmt, arg);
    DPRN(fmt, arg);
}

void bleResponseCallback(const char* resp) {
    espConn.send(resp);
}


void printHeapStats(){ 
    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);
    espConn.send("[info] heap: %lu / %lu", heap_stats.current_size, heap_stats.max_size);
}

void toggleStatusLed() {
    statusLed = !statusLed;
    printHeapStats();
}

int main()
{
    // statusToggler.attach(toggleStatusLed, 1);
    espConn.init(espDataReceivedCb);
    
    // delay for 30 secs to start esp-conn
    // add callback to notify mqtt connected
    eventQueue.call_in(15000, []()->void {
        bleConn.init(bleResponseCallback, bleDebugPrint);
    });
    // the following doesn't work after ~10 times
    eventQueue.call_every(1000, toggleStatusLed);
    
    eventQueue.dispatch_forever();
    return 0;
}



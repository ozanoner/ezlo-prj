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

// https://os.mbed.com/teams/Bluetooth-Low-Energy/code/BLE_LED/


#include <mbed.h>
#include <mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "HAHardwareDefs.h"
#include "STPM01Driver.h"
#include "SEGGER_RTT.h"
#include "BleConn.h"
#include "HATestButton.h"
#include "SoftPwm1.h";

DigitalOut plug(P0_4, 1);
DigitalOut triacPin(P0_7, 1);
SoftPwm1 triac(triacPin);

SPI spi(NC, STPM01_SDA, STPM01_SCL); // mosi, miso, sclk
DigitalOut cs(STPM01_SCS, 1);
DigitalOut syn(STPM01_SYN, 1);
STPM01Driver pm(spi, cs, syn);

static EventQueue eventQueue;

BleConn bleConn(eventQueue);
static HATestButton testButton(eventQueue);


int main(void)
{
    auto plugCb = [](uint8_t val)-> void {
        plug = val;
        testButton.testLed = val;
    };

    auto triacCb = [](uint8_t val)-> void {
        if(val>0) {
            triac.start_us(100, val);
        }
        else  {
            triac.stop();
        }
        
    }; 

    bleConn.setControlCallbacks(plugCb, triacCb);

    testButton.setPressCb([]()-> void {
        plug = !plug;
    });


    bleConn.init();
    
    auto energyCb = [](const uint32_t* data)->void {
        bleConn.sendEnergyData(data);
    };
    pm.init(energyCb);

    eventQueue.call_every(5000, []()->void {
        pm.read();
    });

   
    eventQueue.dispatch_forever();
}

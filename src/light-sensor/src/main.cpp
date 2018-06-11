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

// original: https://os.mbed.com/teams/Bluetooth-Low-Energy/code/BLE_Button/

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/Gap.h"

// #include "LightSensorService.h"
#include "BleConn.h"
#include "Opt3001.h"

#include "HAProvision.h"
#include "HABleServiceDefs.h"
#include "HAHardwareDefs.h"
#include "SEGGER_RTT.h"

#include "HATestButton.h"

using namespace std;


I2C i2c(P0_12, P0_11);
Opt3001 sensor(i2c);


static EventQueue eventQueue;

HATestButton testButton(eventQueue);
BleConn bleConn(eventQueue);


int main(void)
{
	sensor.init();
    bleConn.init();
    DPRN("[info] init completed\n");

    auto funcReadSensor = [&sensor]()->void{
        auto val = sensor.readLux();
        DPRN("[info] lux:%x\n", val);
        bleConn.updateSensorState(val);

    };
    eventQueue.call_every(5000, funcReadSensor);

    eventQueue.dispatch_forever();
}

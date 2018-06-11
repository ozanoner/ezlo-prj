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

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
// #include "LEDService.h"
#include "BleConn.h"

#include "HATestButton.h"
#include "HAHardwareDefs.h"
#include "HAProvision.h"
#include "SoftPwm1.h"
#include <nrf.h>

// SoftPWM
// https://os.mbed.com/users/komaida424/code/SoftPWM/




static EventQueue eventQueue;

HATestButton testButton(eventQueue);

// DigitalOut led(testButton.testLed);
DigitalOut led(LED_WHITE_PWM);
const static uint8_t MAX_DUTY=100;
const static uint8_t FREQ_US=100;
SoftPwm1 rLed(led);

BleConn bleConn(eventQueue, MAX_DUTY);

void setLedDuty(uint8_t duty) {
    rLed.start_us(FREQ_US, duty>MAX_DUTY? MAX_DUTY:duty);
}

int main(void)
{
    // setLedDuty(90);

    testButton.setPressCb([]()-> void {
        led = !led;
    });
    bleConn.init();
    bleConn.setDataWrittenCb([](uint8_t i)-> void {
        led = i;
    });
    
    eventQueue.dispatch_forever();

}


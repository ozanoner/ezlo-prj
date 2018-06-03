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
#include "BleConn.h"

#include "HATestButton.h"
#include "HAHardwareDefs.h"
#include "HAProvision.h"
#include "SoftPwm1.h"
// #include "SEGGER_RTT.h"

// SoftPWM
// https://os.mbed.com/users/komaida424/code/SoftPWM/




static EventQueue eventQueue;

HATestButton testButton(eventQueue);

// DigitalOut led(testButton.testLed);
DigitalOut rled(RLED_PWM);
SoftPwm1 rledPwm(rled);
DigitalOut gled(GLED_PWM);
SoftPwm1 gledPwm(gled);
DigitalOut bled(BLED_PWM);
SoftPwm1 bledPwm(bled);
const static uint8_t MAX_DUTY=50;
const static uint8_t FREQ_US=100;

BleConn bleConn(eventQueue, MAX_DUTY);

void setLedDuty(uint32_t duty) {

    uint8_t* valptr = reinterpret_cast<uint8_t*>(&duty);
    uint8_t rval = valptr[0]>MAX_DUTY? MAX_DUTY:valptr[0];
    uint8_t gval = valptr[1]>MAX_DUTY? MAX_DUTY:valptr[1];
    uint8_t bval = valptr[2]>MAX_DUTY? MAX_DUTY:valptr[2];

    rledPwm.start_us(FREQ_US, rval);
    gledPwm.start_us(FREQ_US, gval);
    bledPwm.start_us(FREQ_US, bval);

    SEGGER_RTT_printf(0, "[info] rval:(%x) gval:(%x) bval:(%x)", rval, gval, bval);
}

int main(void)
{
    // SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    // setLedDuty(50);

    bleConn.init();
    bleConn.setDataWrittenCb(setLedDuty);
    
    eventQueue.dispatch_forever();

}


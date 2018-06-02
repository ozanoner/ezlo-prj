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

 https://github.com/ARMmbed/mbed-os-example-ble/tree/master/BLE_Button

 */
#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/Gap.h"


#include "BleConn.h"

#include "SEGGER_RTT.h"

#include "HATestButton.h"
#include "HAHardwareDefs.h"
#include "HABleServiceDefs.h"
#include "HAProvision.h"

InterruptIn button1(BTN_BTN1);
InterruptIn button2(BTN_BTN2);

static EventQueue eventQueue;
HATestButton testBtn(eventQueue);
BleConn bleConn(eventQueue);


void buttonPressedCallback(void)
{
    bleConn.updateButtonState(true);
    testBtn.testLed =1;
}

void buttonReleasedCallback(void)
{
    bleConn.updateButtonState(false);
    testBtn.testLed =0;
}

void button2PressedCallback(void)
{
    bleConn.updateButton2State(true);
    testBtn.testLed =1;
}

void button2ReleasedCallback(void)
{
    bleConn.updateButton2State(false);
    testBtn.testLed =0;
}

int main()
{
    bleConn.init();

    button1.fall(buttonPressedCallback);
    button1.rise(buttonReleasedCallback);

    button2.fall(button2PressedCallback);
    button2.rise(button2ReleasedCallback);


    eventQueue.dispatch_forever();

    return 0;
}

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

#include "RgbLedService.h"
#include "RGB.h"
#include <cstring>

#include "HAHardwareDefs.h"

// RGB led(LED2,LED3,LED4);

// PwmOut rLed(P0_8), gLed(P0_19), bLed(P0_20);

DigitalOut led1(STATUS_LED, 1);
InterruptIn testButton(TEST_BTN);


DigitalOut rLed(P0_8, 1), gLed(P0_19, 1), bLed(P0_20, 1);


static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);


const static char     DEVICE_NAME[] = "RGB-LED";
static const uint16_t uuid16_list[] = {RgbLedService::RGBLED_SERVICE_UUID};

RgbLedService *ledServicePtr;


void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising();
}


void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if (params->handle == ledServicePtr->getRGBCharHandle()) {
		// pwmout, smooth transition can be added
		uint32_t color;
		memcpy(&color, params->data, 4);
		// led.setColor(Color(color));
    }
}


void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}


void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);

    ledServicePtr = new RgbLedService(ble);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main(void)
{
    // eventQueue.call_every(1000, []()->void{ 
    //     led1 = !led1;
    //     rLed = !rLed;
    //     gLed = !gLed;
    //     bLed = !bLed;       
    // });

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    // rLed.period(1); rLed.write(1);
    // gLed.period(1); gLed.write(1);
    // bLed.period(1); bLed.write(1);

    testButton.fall([]()->void {
        eventQueue.call([]()->void {
            led1 = !led1;
            rLed = !rLed;
            gLed = !gLed;
            bLed = !bLed;
        });
    });

    eventQueue.dispatch_forever();
}
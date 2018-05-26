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
#include "LEDService.h"

// SoftPWM
// https://os.mbed.com/users/komaida424/code/SoftPWM/



DigitalOut  statusLed(P0_9, 1);
InterruptIn testButton(P0_10);

// DigitalOut  ledBtnDisp(P0_9, 0);
// DigitalOut  ledBtnDisp(P0_8, 0);

// DigitalOut actuatedLED(P0_8, 1);

PwmOut actuatedLED(P0_8);
// PwmOut l2(P0_19);
// PwmOut l3(P0_20);

// InterruptIn button(P0_10);


const static char     DEVICE_NAME[] = "LED";
static const uint16_t uuid16_list[] = {LEDService::LED_SERVICE_UUID};

LEDService *ledServicePtr;

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);


void toggleActLed() {
    // actuatedLED = !actuatedLED; 
    // ledBtnDisp = !ledBtnDisp;
}


void buttonPressedCallback(void)
{
    // eventQueue.call(setButtonLed, actOnOff);
    // eventQueue.call(toggleActLed);
    // ledBtnDisp = 1;
}

void buttonReleasedCallback(void)
{
    // eventQueue.call(setButtonLed, false);
    // ledBtnDisp = 0;
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising();
}

void blinkCallback(void)
{
    // ledBtnDisp = !ledBtnDisp;
}


void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == ledServicePtr->getValueHandle()) && (params->len == 1)) {
        actuatedLED = *(params->data);
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

    bool initialValueForLEDCharacteristic = false;
    ledServicePtr = new LEDService(ble, initialValueForLEDCharacteristic);

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

    // eventQueue.call_every(1000, blinkCallback);


    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    // button.fall(buttonPressedCallback);
    // button.rise(buttonReleasedCallback);

    actuatedLED.period(0.000001);
    actuatedLED = 1;

    // l2.period(0.000001);
    // l2 = 1;

    // l3.period(0.000001);
    // l3 = 1;


    /* test */
    testButton.fall([]()-> void{
        eventQueue.call(Callback<void()>([]()-> void { 
            statusLed=!statusLed; 
            actuatedLED = !actuatedLED;
        } ));
    });

    eventQueue.dispatch_forever();

}


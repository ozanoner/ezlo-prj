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
#include "ButtonService.h"
// #include "equeue.h"
   

const Gap::Address_t  BLE_address_BE       = {0xCC, 0x00, 0x00, 0xE1, 0x80, 0xA1};

DigitalOut  led1(LED1, 1);
DigitalOut  ledBtnDisp(LED2, 1);
DigitalOut ledConnected(LED3, 1);
InterruptIn button(BUTTON1);

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

const static char     DEVICE_NAME[] = "Button";
static const uint16_t uuid16_list[] = {ButtonService::BUTTON_SERVICE_UUID};

ButtonService *buttonServicePtr;

void setButtonLed(bool on) {
    ledBtnDisp = on?0:1;
}


void buttonPressedCallback(void)
{
    eventQueue.call(Callback<void(bool)>(buttonServicePtr, &ButtonService::updateButtonState), true);
    eventQueue.call(setButtonLed, true);
}

void buttonReleasedCallback(void)
{
    eventQueue.call(Callback<void(bool)>(buttonServicePtr, &ButtonService::updateButtonState), false);
    eventQueue.call(setButtonLed, false);
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising(); // restart advertising
    ledConnected = 1;
}


void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
    ledConnected = 0;
}

void blinkCallback(void)
{
    led1 = !led1; 
}

void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/*
void printMacAddress()
{
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);

    printf("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
        printf("%02x:", address[i]);
    }
    printf("%02x\r\n", address[0]);
}
*/

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
    ble.gap().onConnection(connectionCallback);

    button.fall(buttonPressedCallback);
    button.rise(buttonReleasedCallback);

    /* Setup primary service. */
    buttonServicePtr = new ButtonService(ble, false /* initial value for button pressed */);

    /* setup advertising */
    ble.gap().setAddress(BLEProtocol::AddressType::PUBLIC, BLE_address_BE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();


    // printMacAddress();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    eventQueue.call_every(500, blinkCallback);

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}

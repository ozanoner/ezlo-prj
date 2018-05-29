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


#include "SEGGER_RTT.h"
#define DPRN(...) SEGGER_RTT_printf(0, __VA_ARGS__)


#include "HABleServiceDefs.h"
#include "HAProvision.h"
#define DEV_PROVISION_ID BTN1_ID2

static const uint8_t DEVICE_NAME[] = "Btn1-2";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t uuid16_list[] = {BUTTON1_SERVICE_UUID};

// DigitalOut  led1(LED1, 1);
// DigitalOut  ledBtnDisp(LED2, 1);
// DigitalOut ledConnected(LED3, 1);
// InterruptIn button(BUTTON1);

DigitalOut  ledBtnDisp(P0_9, 1);
InterruptIn button(P0_10);
// InterruptIn button1(P0_30);
// InterruptIn button2(P0_31);

// InterruptIn button(P0_30);

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);


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
    DPRN("[info] disconnected\n");
    // ledConnected = 1;
}


void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
    // ledConnected = 0;
    DPRN("[info] connected\n");
}

void blinkCallback(void)
{
    // led1 = !led1; 
    // led2 = !led2; 
    ledBtnDisp = !ledBtnDisp;
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

// https://os.mbed.com/users/yasuyuki/code/mbed_BLE/docs/tip/main_8cpp_source.html
    if(ble.gap().setTxPower(4)!=BLE_ERROR_NONE) {

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
    ble.gap().setAddress(BLEProtocol::AddressType::PUBLIC, BLE_NW_ADDR);
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
    // eventQueue.call_every(1000, blinkCallback);

    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);

    DPRN("started");

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}

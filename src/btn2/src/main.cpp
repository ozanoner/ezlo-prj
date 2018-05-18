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
#include "ButtonService.h"
// #include "equeue.h"

#include <iostream>

using namespace std;

// chip led p0.09
// chip btn1 p0.30 user
// chip btn2 p0.31 user
// chip btn-free (debug) p0.10


DigitalOut  statusLed(P0_9, 1);
InterruptIn button1(P0_30);
InterruptIn button2(P0_31);
InterruptIn testButton(P0_10);



// DigitalOut  led1(LED1, 1);
// InterruptIn button1(BUTTON1);
// InterruptIn button2(BUTTON2);

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

const static char     DEVICE_NAME[] = "Button";
static const uint16_t uuid16_list[] = {ButtonService::BUTTON_SERVICE_UUID};

ButtonService *buttonServicePtr;

void button1PressedCallback(void)
{
    eventQueue.call(Callback<void(uint16_t, bool)>(buttonServicePtr,\
        &ButtonService::updateButtonState),\
        ButtonService::BUTTON1_STATE_CHARACTERISTIC_UUID, true);
}

void button1ReleasedCallback(void)
{
    eventQueue.call(Callback<void(uint16_t, bool)>(buttonServicePtr,\
        &ButtonService::updateButtonState),\
        ButtonService::BUTTON1_STATE_CHARACTERISTIC_UUID, false);
}

void button2PressedCallback(void)
{
    eventQueue.call(Callback<void(uint16_t, bool)>(buttonServicePtr,\
        &ButtonService::updateButtonState),\
        ButtonService::BUTTON2_STATE_CHARACTERISTIC_UUID, true);
}

void button2ReleasedCallback(void)
{
    eventQueue.call(Callback<void(uint16_t, bool)>(buttonServicePtr,\
        &ButtonService::updateButtonState),\
        ButtonService::BUTTON2_STATE_CHARACTERISTIC_UUID, false);
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising(); // restart advertising
}

void blinkCallback(void)
{
    // led1 = !led1; /* Do blinky on LED1 to indicate system aliveness. */
}

void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

void printMacAddress()
{
    /* Print out device MAC address to the console*/
    Gap::AddressType_t addr_type;
    Gap::Address_t address;
    BLE::Instance().gap().getAddress(&addr_type, address);

    printf("DEVICE MAC ADDRESS: ");
    for (int i = 5; i >= 1; i--){
        printf("%02x:", address[i]);
    }
    printf("%02x\r\n", address[0]);

/*
    cout << "mac address:";
    for(int i=0; i<6; ++i)
        cout << address[i];
    cout << std::hex << endl;
    */
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

    // button1.fall(button1PressedCallback);
    // button1.rise(button1ReleasedCallback);
    // button2.fall(button2PressedCallback);
    // button2.rise(button2ReleasedCallback);

    /* Setup primary service. */
    buttonServicePtr = new ButtonService(ble, false /* initial value for button pressed */);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

    printMacAddress();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main()
{
    // eventQueue.call_every(500, blinkCallback);



    /* test code */
    statusLed = 1;

    button1.fall([]()-> void{
        eventQueue.call(Callback<void()>([]()-> void { statusLed=!statusLed; } ));
    });

    button2.fall([]()-> void{
        eventQueue.call(Callback<void()>([]()-> void { statusLed=!statusLed; } ));
    });

    testButton.fall([]()-> void{
        eventQueue.call(Callback<void()>([]()-> void { statusLed=!statusLed; } ));
    });


    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();

    return 0;
}
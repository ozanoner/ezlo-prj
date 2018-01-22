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

#include "mbed.h"
#include "ble/BLE.h"
#include "ButtonService.h"

DigitalOut  led1(LED1);
InterruptIn button1(BUTTON1);
InterruptIn button2(BUTTON2);


const static char     DEVICE_NAME[] = "Button";
static const uint16_t uuid16_list[] = {ButtonService::BUTTON_SERVICE_UUID};

enum {
    RELEASED = 0,
    PRESSED,
    IDLE
};
static uint8_t button1State = IDLE;
static uint8_t button2State = IDLE;

static ButtonService *buttonServicePtr;

void button1PressedCallback(void)
{
    /* Note that the button1PressedCallback() executes in interrupt context, so it is safer to access
     * BLE device API from the main thread. */
    button1State = PRESSED;
}
void button2PressedCallback(void)
{
    /* Note that the button1PressedCallback() executes in interrupt context, so it is safer to access
     * BLE device API from the main thread. */
    button2State = PRESSED;
}

void button1ReleasedCallback(void)
{
    /* Note that the button1ReleasedCallback() executes in interrupt context, so it is safer to access
     * BLE device API from the main thread. */
    button1State = RELEASED;
}
void button2ReleasedCallback(void)
{
    /* Note that the button1ReleasedCallback() executes in interrupt context, so it is safer to access
     * BLE device API from the main thread. */
    button2State = RELEASED;
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising();
}

void periodicCallback(void)
{
    led1 = !led1; /* Do blinky on LED1 to indicate system aliveness. */
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/**
 * Callback triggered when the ble initialization process has finished
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

    /* Setup primary service */
    buttonServicePtr = new ButtonService(ble, false /* initial value for button pressed */);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

}

int main(void)
{
    led1 = 1;
    Ticker ticker;
    ticker.attach(periodicCallback, 1);
    button1.fall(button1PressedCallback);
    button1.rise(button1ReleasedCallback);

	button2.fall(button2PressedCallback);
    button2.rise(button2ReleasedCallback);


    BLE &ble = BLE::Instance();
    ble.init(bleInitComplete);

    /* SpinWait for initialization to complete. This is necessary because the
     * BLE object is used in the main loop below. */
    while (ble.hasInitialized()  == false) { /* spin loop */ }

    while (true) {
        if (button1State != IDLE) {
            buttonServicePtr->updateButtonState(ButtonService::BUTTON1_STATE_CHARACTERISTIC_UUID, button1State);
            button1State = IDLE;
        }
		if (button2State != IDLE) {
			buttonServicePtr->updateButtonState(ButtonService::BUTTON2_STATE_CHARACTERISTIC_UUID, button2State);
			button2State = IDLE;
		}


        ble.waitForEvent();
    }
}

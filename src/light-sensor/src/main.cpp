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

#include "LightSensorService.h"
#include "Opt3001.h"

#include "HAProvision.h"
#include "HABleServiceDefs.h"
#include "HAHardwareDefs.h"
#include "SEGGER_RTT.h"

#include "HATestButton.h"

I2C i2c(P0_12, P0_11);
Opt3001 sensor(i2c);

#define DEV_PROVISION_ID LS_ID8


static const uint8_t DEVICE_NAME[] = "LS_ID8";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t SERVICE_UUID_LIST[] = {LIGHT_SERVICE_UUID};

using namespace std;
static LightSensorService *sensorServicePtr;

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

HATestButton testButton(eventQueue);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising();
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

    // if(ble.gap().setTxPower(4)!=BLE_ERROR_NONE) {
    //     DPRN("[error] setTxPower");
    // }

    /* Setup primary service */
    sensorServicePtr = new LightSensorService(ble);

    /* setup advertising */
    ble.gap().setAddress(BLEProtocol::AddressType::PUBLIC, BLE_NW_ADDR);

    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)SERVICE_UUID_LIST, sizeof(SERVICE_UUID_LIST));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

}

// Opt3001Base sensor; // dummy values

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

int main(void)
{
	sensor.init();
    auto funcReadSensor = [&sensor]()->void{
        auto val = sensor.readLux();
        DPRN("[info] lux:%x\n", val);
        sensorServicePtr->updateSensorState(val);
    };
    eventQueue.call_every(5000, funcReadSensor);

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();
}

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


#include <mbed.h>
#include <mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "DimmerService.h"
#include "HAHardwareDefs.h"
#include "STPM01Driver.h"
#include "SEGGER_RTT.h"

DigitalOut led1(LED1, 0);
PwmOut actuatedLED(LED2);
SPI spi(NC, STPM01_SDA, STPM01_SCL); // mosi, miso, sclk
DigitalOut cs(STPM01_SCS, 1);
DigitalOut syn(STPM01_SYN, 1);
STPM01Driver pm(spi, cs, syn);

const static char     DEVICE_NAME[] = "Dimmer";
static const uint16_t uuid16_list[] = {DimmerService::DIMMER_SERVICE_UUID};

DimmerService *dimmerService;

static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    BLE::Instance().gap().startAdvertising();
}

void blinkCallback(void)
{
    led1 = !led1; /* Do blinky on LED1 to indicate system aliveness. */
}

void onDataWrittenCallback(const GattWriteCallbackParams *params) {
    if ((params->handle == dimmerService->getValueHandle()) && (params->len == 1)) {
		// pwmout, smooth transition can be added
        actuatedLED = 1 - ((float)(*(params->data)))/255;
    }
}

/**
 * This function is called when the ble initialization process has failed
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

	actuatedLED.period(0.02); // 50hz
	actuatedLED = 1; // off=0% duty

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

    dimmerService = new DimmerService(ble);

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
    // eventQueue.call_every(500, blinkCallback);
    pm.init();

    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.call_every(1000, []()->void {
        pm.read();
    });

    eventQueue.dispatch_forever();
}

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


 // TRY THIS: https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-ble-LEDBlinker/file/bf9a45219fe2/source/main.cpp/


#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "GwDevice.h"

#include "SEGGER_RTT.h"


/*
DiscoveredCharacteristic ledCharacteristic;




void discoveryTerminationCallback(Gap::Handle_t connectionHandle) {
    printf("terminated SD for handle %u\r\n", connectionHandle);
}

void serviceDiscoveryCallback(const DiscoveredService *service) {
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
        printf("S UUID-%x attrs[%u %u]\r\n", service->getUUID().getShortUUID(), service->getStartHandle(), service->getEndHandle());
    } else {
        printf("S UUID-");
        const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
        for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++) {
            printf("%02X", longUUIDBytes[i]);
        }
        printf(" attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
    }
}
*/


// void characteristicDiscoveryCallback(const DiscoveredCharacteristic *characteristicP) {
//     printf("  C UUID-%x valueAttr[%u] props[%x]\r\n", characteristicP->getUUID().getShortUUID(),
//         characteristicP->getValueHandle(), (uint8_t)characteristicP->getProperties().broadcast());
//     if (characteristicP->getUUID().getShortUUID() == 0xa001) { /* !ALERT! Alter this filter to suit your device. */
//       //printf("  C UUID-%x valueAttr[%u] props[%x]\r\n", characteristicP->getShortUUID(), characteristicP->getValueHandle(), (uint8_t)characteristicP->getProperties().broadcast());
//       ledCharacteristic = *characteristicP;
//       triggerOp = READ;
//     }
// }

// void connectionCallback(const Gap::ConnectionCallbackParams_t *params) {
//     printf("in conn callback\n");
//   uint16_t LED_SERVICE_UUID = 0xA000;
//   uint16_t LED_STATE_CHARACTERISTIC_UUID = 0xA001;

//   if (params->role == Gap::CENTRAL) {
//     BLE &ble = BLE::Instance();
//     ble.gattClient().onServiceDiscoveryTermination(discoveryTerminationCallback);
//     ble.gattClient().launchServiceDiscovery(params->handle, serviceDiscoveryCallback,
//         characteristicDiscoveryCallback, LED_SERVICE_UUID, LED_STATE_CHARACTERISTIC_UUID);
//   }
// }

// void triggerToggledWrite(const GattReadCallbackParams *response) {
//   if (response->handle == ledCharacteristic.getValueHandle()) {
// #if 0
//     printf("triggerToggledWrite: handle %u, offset %u, len %u\r\n", response->handle, response->offset, response->len);
//     for (unsigned index = 0; index < response->len; index++) {
//       printf("%c[%02x]", response->data[index], response->data[index]);
//     }
//     printf("\r\n");
// #endif

//     toggledValue = response->data[0] ^ 0x1;
//     triggerOp = WRITE;
//   }
// }

// void triggerRead(const GattWriteCallbackParams *response) {
//   if (response->handle == ledCharacteristic.getValueHandle()) {
//     triggerOp = READ;
//   }
// }

// /**
//  * This function is called when the ble initialization process has failled
//  */
// void onBleInitError(BLE &ble, ble_error_t error)
// {
//     /* Initialization error handling should go here */
// }

// /**
//  * Callback triggered when the ble initialization process has finished
//  */
// void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
// {
//     BLE&        ble   = params->ble;
//     ble_error_t error = params->error;

//     if (error != BLE_ERROR_NONE) {
//         /* In case of error, forward the error handling to onBleInitError */
//         onBleInitError(ble, error);
//         return;
//     }

//     /* Ensure that it is the default instance of BLE */
//     if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
//         return;
//     }

//     // Set BT Address
//     ble.gap().setAddress(BLEProtocol::AddressType::PUBLIC, BLE_address_BE);

//     ble.gap().onConnection(connectionCallback);
//     ble.gap().onDisconnection(disconnectionCallback);

//     ble.gattClient().onDataRead(triggerToggledWrite);
//     ble.gattClient().onDataWritten(triggerRead);

//     ble.gap().setScanParams(SCAN_INT, SCAN_WIND);
//     // ble.gap().startScan(advertisementCallback);
//     perScanEnabled = true;


// }


int main()
{
    GwDevice gap_device;
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);

    while (1) {
        gap_device.run();
        // wait_ms(TIME_BETWEEN_MODES_MS);
        // printf("\r\nStarting next GAP demo mode\r\n");
    };

    return 0;
}



/*
1- startscan -> advertisementCallback
2- connect -> connectionCallback
3- launchServiceDiscovery serviceDiscoveryCallback, characteristicDiscoveryCallback
4- charDisc -> set ledCharacteristic & read ledStatus
5- then triggers read<->write continously (onDataRead & onDataWrite) based on filter ledCharacteristic.getValueHandle

*/

/*
TODO:
1- characteristic handlers for each device (btn, led, sensor etc) based on service & char-uuid
2- serial connection to the esp32

*/


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

#ifndef __BLE_DIMMER_SERVICE_H__
#define __BLE_DIMMER_SERVICE_H__

#include <mbed.h>
#include "ble/BLE.h"

class DimmerService {
public:
    const static uint16_t DIMMER_SERVICE_UUID              = 0xA200;
    const static uint16_t DIMMER_STATE_CHARACTERISTIC_UUID = 0xA201;

    DimmerService(BLEDevice &_ble) :
        ble(_ble), dimmerState(DIMMER_STATE_CHARACTERISTIC_UUID, 0)
    {
        GattCharacteristic *charTable[] = {&dimmerState};
        GattService         dimmerService(DIMMER_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(dimmerService);
    }

    GattAttribute::Handle_t getValueHandle() const {
        return dimmerState.getValueHandle();
    }

private:
    BLEDevice                         &ble;
    ReadWriteGattCharacteristic<uint8_t>  dimmerState;
};

#endif /* #ifndef __BLE_DIMMER_SERVICE_H__ */

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

#ifndef LightSensorService_h_
#define LightSensorService_h_

class LightSensorService {
public:
    const static uint16_t LIGHT_SERVICE_UUID              = 0xA600;
    const static uint16_t LIGHT_STATE_CHARACTERISTIC_UUID = 0xA601;

    LightSensorService(BLEDevice &_ble) :
        ble(_ble), lightState(LIGHT_STATE_CHARACTERISTIC_UUID, nullptr)
    {
        GattCharacteristic *charTable[] = {&lightState};
        GattService         ledService(LIGHT_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(ledService);
    }

    GattAttribute::Handle_t getValueHandle() const {
        return lightState.getValueHandle();
    }

private:
    BLEDevice                         &ble;
    ReadOnlyGattCharacteristic<uint32_t>  lightState;
};

#endif /* #ifndef __BLE_LED_SERVICE_H__ */

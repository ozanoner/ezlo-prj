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

#ifndef __BLE_BUTTON_SERVICE_H__
#define __BLE_BUTTON_SERVICE_H__

#include "HABleServiceDefs.h"

class ButtonService {
public:
    // const static uint16_t BUTTON_SERVICE_UUID              = 0xA000;
	// const static uint16_t BUTTON1_STATE_CHARACTERISTIC_UUID = 0xA001;
    // const static uint16_t BUTTON2_STATE_CHARACTERISTIC_UUID = 0xA002;

    ButtonService(BLE &_ble, bool buttonPressedInitial) : ble(_ble),
		button1State(BUTTON1_STATE_CHARACTERISTIC_UUID, &buttonPressedInitial, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
		button2State(BUTTON2_STATE_CHARACTERISTIC_UUID, &buttonPressedInitial, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&button1State, &button2State};
        GattService buttonService(BUTTON2_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(buttonService);
    }

    void updateButtonState(uint16_t btnStateUuid, bool newState) {
		ReadOnlyGattCharacteristic<bool>& btnState = btnStateUuid == BUTTON1_STATE_CHARACTERISTIC_UUID? button1State:button2State;
        ble.gattServer().write(btnState.getValueHandle(), (uint8_t *)&newState, sizeof(bool));
    }

private:
    BLE                              &ble;
	ReadOnlyGattCharacteristic<bool>  button1State;
    ReadOnlyGattCharacteristic<bool>  button2State;
};

#endif /* #ifndef __BLE_BUTTON_SERVICE_H__ */

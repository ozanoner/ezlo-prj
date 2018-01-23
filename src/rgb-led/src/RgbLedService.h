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

// color class https://os.mbed.com/users/sillevl/code/RGB-fun/

#ifndef RgbLedService_h_
#define RgbLedService_h_

class RgbLedService {
public:
    const static uint16_t RGBLED_SERVICE_UUID              = 0xA500;
	const static uint16_t RGBLED_RGB_STATE_CHAR_UUID = 0xA501; //

    const static uint16_t RGBLED_BR_STATE_CHAR_UUID = 0xA504; // brightness

    RgbLedService(BLEDevice &_ble) :
        ble(_ble),
		rState(RGBLED_RGB_STATE_CHAR_UUID, nullptr),
		brState(RGBLED_BR_STATE_CHAR_UUID, nullptr)
    {
        GattCharacteristic *charTable[] = {&rState, &brState};
        GattService         rgbledService(RGBLED_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.addService(rgbledService);
    }

    GattAttribute::Handle_t getRGBCharHandle() const {
        return rState.getValueHandle();
    }

	GattAttribute::Handle_t getBrightnessCharHandle() const {
        return brState.getValueHandle();
    }

private:
    BLEDevice                         &ble;
	ReadWriteGattCharacteristic<uint32_t>  rState;
    ReadWriteGattCharacteristic<uint8_t>  brState;
};

#endif /* #ifndef RgbLedService_h_ */

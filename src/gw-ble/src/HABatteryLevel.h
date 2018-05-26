#ifndef HABatteryLevel_h_
#define HABatteryLevel_h_

#include "HAStateReadonly.h"
#include "inttypes.h"

class HABatteryLevel: HAStateReadonly<uint8_t> {
public:
    void getValue() {
        // send command to read value
    }
};

#endif
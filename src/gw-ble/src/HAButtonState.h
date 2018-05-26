#ifndef HAButtonState_h_
#define HAButtonState_h_

#include "HAStateReadonly.h"
#include "inttypes.h"

class HAButtonState: public HAStateReadonly<bool> {
public:
    void getValue() {
        // send command to read value
    }
};

#endif
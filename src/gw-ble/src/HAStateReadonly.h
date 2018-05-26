#ifndef HAStateReadonly_h_
#define HAStateReadonly_h_

#include "inttypes.h"

enum class HADevStateType: uint8_t {
    SENSOR_ON_OFF=1, 
    ACTUATOR_ON_OFF=1, 
};

template <typename Tval>
class HAStateReadonly {
protected:
    HADevStateType type;
    int id;
    Tval val;
public:
    int getStateId() const { return id; }
    // send commands to the device to read the state
    // void getValue()=0;
};



#endif
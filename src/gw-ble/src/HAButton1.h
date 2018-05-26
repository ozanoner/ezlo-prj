#ifndef HAButton1_h_
#define HAButton1_h_

#include "HADevShadow.h"
#include "HAButtonState.h"
#include "HABatteryLevel.h"
#include <string>
#include "HABleServiceDefs.h"
#include <utility>

using namespace std;

class HAButton1: public HADevShadow {
public:
    HAButtonState btn1;
    HABatteryLevel battery;

    string toString();
    HAButton1() { }
    HAButton1(HADevShadow&& old): HADevShadow(std::forward<HADevShadow>(old)) { }
};


string HAButton1::toString() {
    string s;
    return s;
}



#endif
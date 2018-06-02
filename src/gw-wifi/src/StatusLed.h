#ifndef StatusLed_h_
#define StatusLed_h_

#include <Arduino.h>

#define STATUS_LED 33

class StatusLed {
private:
    bool state;
    unsigned long nextUpdate;
    uint16_t period;
public:
    StatusLed(): period(0) {
        pinMode(STATUS_LED, OUTPUT);
        this->setState(true);
    }
    // 0 disables blink
    void setBlinkPeriod(uint16_t blinkPeriod_ms=0) {
        this->period = blinkPeriod_ms;
        this->nextUpdate = millis()+this->period;
    }
    void update() {
        // blink disabled
        if(this->period == 0)
            return;
        if(this->nextUpdate < millis()) {
            this->setState(!this->state);
            this->nextUpdate = millis()+this->period;
        }
    }
    void setState(bool s) {
        this->state = s;
        digitalWrite(STATUS_LED, state);
    }
    
};

#endif
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
    StatusLed(): period(0), state(false) {
        pinMode(STATUS_LED, OUTPUT);
        digitalWrite(STATUS_LED, this->state);
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
        auto now = millis();
        if(this->nextUpdate < now) {
            this->state = !this->state;
            digitalWrite(STATUS_LED, this->state);
            this->nextUpdate = now+this->period;
        }
    }
};

#endif
#ifndef ProgressDisp_h_
#define ProgressDisp_h_

#include <Arduino.h>

class ProgressDisp {
private:
    volatile bool enabled;
    unsigned long next;
    uint8_t cnt;
public:
    ProgressDisp(): enabled(false), cnt(0) { }
    void enable() { 
        this->enabled = true;
        next = millis()+1000;
    } 
    void disable() { 
        this->enabled = false;
        this->cnt =0;
    }
    void update() {
        if(!this->enabled)
            return;
        auto now = millis();
        if(next>now)
            return;
        if(++cnt%30 == 0) {
            Serial.println();
        }
        else {
            Serial.print(".");
        }
        next = now+1000;
    } 
};

#endif
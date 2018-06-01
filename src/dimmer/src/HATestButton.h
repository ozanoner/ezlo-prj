#ifndef HATestButton_h_
#define HATestButton_h_

#include <mbed.h>
#include <mbed_events.h>
#include "HAHardwareDefs.h"
#include "EventQueue.h"
#include <functional>


class HATestButton: private mbed::NonCopyable<BleConn> {
private:
    void onButtonPress() {
        this->evq.call([&]()-> void{
            this->testLed = !this->testLed;
            if(this->fallCb!=nullptr)
                this->fallCb();
        });
    }
    void onButtonRelease() {
        if(this->riseCb!=nullptr) {
            this->evq.call([&]()-> void {
                this->riseCb();
            });
        }
    }
    EventQueue& evq;
    Callback<void()> fallCb, riseCb;
public:
    DigitalOut testLed;
    InterruptIn testButton;

    HATestButton(EventQueue& evq): evq(evq), fallCb(nullptr), riseCb(nullptr),
        testLed(STATUS_LED, 0), testButton(TEST_BTN) { 
        testButton.fall(this, &HATestButton::onButtonPress);
        testButton.rise(this, &HATestButton::onButtonRelease);
    }
    void setPressCb(Callback<void()> f) {
       this->fallCb = f;
    }
    void setReleaseCb(Callback<void()> f) {
        this->riseCb = f;
    }

};


#endif
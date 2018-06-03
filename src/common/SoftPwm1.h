#ifndef SoftPwm1_h_
#define SoftPwm1_h_

#include <mbed.h>
#include <functional>

class SoftPwm1 {
public:
    SoftPwm1(DigitalOut& pin): pin(pin) { }
    void start_us(uint32_t period_us, uint8_t duty) {
        this->width = period_us * (duty>100?100:duty) / 100;
        this->ticker.attach_us(callback(this, &SoftPwm1::tickerInterrupt), period_us);
    }

    void stop() {
        this->ticker.detach();
    }

protected:
    Ticker ticker;
    Timeout timeout;
    DigitalOut& pin;
    uint32_t width;

    void tickerInterrupt() {
        this->pin=1;
        this->timeout.attach_us([&]()-> void {
            pin=0;
        }, this->width);
    }
};

#endif


#include "HeartBeatEffect.h"

HeartBeatEffect::HeartBeatEffect(RGB* led) : Effect(led)
{
    updown = true;
    brightness = 0;
}

void HeartBeatEffect::run()
{
    if(updown) {
        brightness += 10;
    } else {
        brightness -= 20;
    }

    if(brightness > 255 || brightness < 0) changeDirection();
    brightness = saturate(brightness);

    setLight(brightness);
    wait(0.02 * speed);
}

void HeartBeatEffect::setLight(int value)
{

    led->setColor( value << 16);
}

void HeartBeatEffect::changeDirection()
{
    updown = !updown;
}

int HeartBeatEffect::saturate(int value)
{
    if(value > 255) value = 255;
    if(value < 0) value = 0;
    return value;
}

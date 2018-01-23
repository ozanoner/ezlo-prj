
#include "Effect.h"

Effect::Effect(RGB* led)
{
    this->led = led;
    speed = 1.0;
    reset();
}

Effect::~Effect()
{
    delete led;
}

void Effect::setSpeed(float speed)
{
    this->speed = speed;
}

float Effect::getSpeed()
{
    return speed;
}

void Effect::reset()
{
    led->setColor(0);
}

void Effect::flash(Color * color, float onTime, float offTime)
{
    flash(color->getHex(), onTime, offTime);
}

void Effect::flash(int color, float onTime, float offTime)
{
    led->setColor(color);
    wait(onTime * speed);
    led->setColor(RGB::OFF);
    wait(offTime * speed);
}

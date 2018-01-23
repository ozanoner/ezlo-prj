
#include "RGB.h"

#ifndef EFFECT_H
#define EFFECT_H

class Effect
{
protected:

    float speed;
    RGB* led;

    void flash(Color * color, float onTime, float offTime);
    void flash(int color, float onTime, float offTime);

public:

    Effect(RGB* led);
    virtual ~Effect();

    void setSpeed(float speed);
    float getSpeed();

    virtual void run() = 0;
    virtual void reset();

};

#endif
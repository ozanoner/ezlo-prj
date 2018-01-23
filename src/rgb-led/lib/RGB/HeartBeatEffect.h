
#include "Effect.h"

#ifndef HEARTBEATEFFECT_H
#define HEARTBEATEFFECT_H

class HeartBeatEffect : public Effect
{

    int updown;
    int brightness;

    void setLight(int value);
    int saturate(int value);
    void changeDirection();

public:

    HeartBeatEffect(RGB* led);

    virtual void run();
};

#endif
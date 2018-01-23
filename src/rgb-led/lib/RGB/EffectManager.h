
#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include "Effect.h"
#include "CycleEffect.h"
#include "PoliceEffect.h"
#include "HeartBeatEffect.h"

class EffectManager
{

    RGB* led;
    Effect* effect;

    int effectIndex;

public:
    EffectManager(RGB* led);
    virtual ~EffectManager();

    void run();
    void next();

};

#endif

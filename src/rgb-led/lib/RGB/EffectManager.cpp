
#include "EffectManager.h"


EffectManager::EffectManager(RGB* led)
{
    this->led = led;
    effect = new CycleEffect(led);
}

EffectManager::~EffectManager()
{
    delete effect;
}


void EffectManager::next()
{
    effectIndex++;
    if(effectIndex > 3) effectIndex = 0;
    delete effect;
    switch(effectIndex) {
        case 0:
            effect = new PoliceEffect(led);
            break;
        case 1:
            effect = new HeartBeatEffect(led);
            break;
        case 3:
            effect = new CycleEffect(led);
            break;
    }
}

void EffectManager::run()
{
    effect->run();
}
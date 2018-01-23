
#include "Effect.h"
#include "Color.h"

#ifndef CYCLEEFFECT_H
#define CYCLEEFFECT_H

class CycleEffect : public Effect
{

    static Color::Colors COLORS[];
    int index;

public:

    CycleEffect(RGB* led);

    virtual void run();
};

#endif

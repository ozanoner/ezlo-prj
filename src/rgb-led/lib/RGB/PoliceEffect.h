
#include "Effect.h"

#ifndef POLICEEFFECT_H
#define POLICEEFFECT_H

class PoliceEffect : public Effect
{

public:

    PoliceEffect(RGB* led);

    virtual void run();
};

#endif
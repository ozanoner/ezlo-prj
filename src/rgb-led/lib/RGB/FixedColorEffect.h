#include "Effect.h"

#ifndef FIXEDCOLOREFFECT_H
#define FIXEDCOLOREFFECT_H

class FixedColorEffect : public Effect
{

    Color color;

public:
	FixedColorEffect(RGB* led, Color& color);
    FixedColorEffect(RGB* led, Color&& color);
    virtual ~FixedColorEffect();
    virtual void run();
};


#endif

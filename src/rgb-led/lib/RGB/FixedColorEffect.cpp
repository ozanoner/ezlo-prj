
#include "FixedColorEffect.h"

FixedColorEffect::FixedColorEffect(RGB* led, Color& color) : Effect(led)
{
    this->color = color;
}
FixedColorEffect::FixedColorEffect(RGB* led, Color&& color) : Effect(led)
{
    this->color = std::move(color);
}


FixedColorEffect::~FixedColorEffect()
{
    //delete color;
}

void FixedColorEffect::run()
{
    led->setColor(color);
}

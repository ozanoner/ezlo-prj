
#include "CycleEffect.h"

Color::Colors CycleEffect::COLORS[7] = {Color::RED, Color::GREEN, Color::BLUE, Color::CYAN, Color::MAGENTA, Color::YELLOW, Color::WHITE};

CycleEffect::CycleEffect(RGB* led) : Effect(led)
{
}

void CycleEffect::run()
{
    index++;
    if(index >= 7) index = 0;

    led->setColor(COLORS[index]);
    wait (1.0 * speed);
}
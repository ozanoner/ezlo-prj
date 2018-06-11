
#include "PoliceEffect.h"

PoliceEffect::PoliceEffect(RGB* led) : Effect(led)
{

}

void PoliceEffect::run()
{
    flash(Color::RED, 0.1, 0.1);
    flash(Color::RED, 0.1, 0.1);
    wait(0.2 * speed);
    flash(Color::BLUE, 0.1, 0.1);
    flash(Color::BLUE, 0.1, 0.1);
    wait(0.2 * speed);

}


#include "RGB.h"

RGB::RGB(PinName r_pin, PinName g_pin, PinName b_pin)
{
    r_out = new PwmOut(r_pin);
	r_out->period_ms(10);
    g_out = new PwmOut(g_pin);
	g_out->period_ms(10);
    b_out = new PwmOut(b_pin);
	b_out->period_ms(10);
    off();
}

RGB::~RGB()
{
    delete r_out;
    delete g_out;
    delete b_out;
}

void RGB::setColor(Color& color)
{
    this->color = color;
    setPwmColor(color.getRed(), r_out);
    setPwmColor(color.getGreen(), g_out);
    setPwmColor(color.getBlue(), b_out);
}

void RGB::setColor(Color&& color)
{
    this->color = std::move(color);
    setPwmColor(color.getRed(), r_out);
    setPwmColor(color.getGreen(), g_out);
    setPwmColor(color.getBlue(), b_out);
}

void RGB::setColor(int color)
{
    setColor(Color(color));
}

Color RGB::getColor() const
{
    return color;
}

void RGB::off()
{
    setColor(Color(0));
}

void RGB::setPwmColor(int value, PwmOut* output)
{
    output->write(((~value) & 0xFF) / 255.0f);
}

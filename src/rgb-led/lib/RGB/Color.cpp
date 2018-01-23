
#include "Color.h"


Color::Color(int red, int green, int blue)
{
    setColor(red, green, blue);
}

Color::Color(float red, float green, float blue)
{
    setColor((int) red * 255.0f, (int) green * 255.0f, (int) blue * 255.0f);
}

Color::Color(int color)
{
    setColor((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

int Color::getHex()
{
    return ((red << 16) || (green << 8) || (blue << 0));
}

int Color::getRed()
{
    return red;
}

int Color::getGreen()
{
    return green;
}

int Color::getBlue()
{
    return blue;
}

void Color::setColor(int red, int green, int blue)
{
    this->red = red;
    this->green = green;
    this->blue = blue;
}
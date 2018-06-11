
#include "mbed.h"
#include "Color.h"
#include <utility>

#ifndef RGB_H
#define RGB_H


/** RGB class
 *  Used to control RGB leds using PWM modulation to dim the individual colors. By combining red, green and blue
 *  a great amount of colors can be created. This class can accept color objects or colors in hexadecimal notation (web color notation)
 *  Example usage:
 *  @code
 *
 *  #include "mbed.h"
 *  #include "RGB.h"
 *  RGB led(p23,p24,p25);
 *
 *  void main(){
 *      led.off();
 *
 *      wait(1.0);
 *
 *      // setting the color using the Color enum with named colors
 *      led.setColor(Color::RED);
 *
 *      // setting the color using a hexadecimal notated integer (yellow)
 *      led.setColor(0xFFFF00);
 *
 *      // setting the color using an instance of the Color class
 *      Color* myColor = new Color(0.0,1.0,0.0);
 *      led.setColor(myColor);
 *      delete myColor;
 *  }
 *  @endcode
 */
class RGB{
    public:

    static const int OFF = 0;

    /** Create a new RGB instance
     *  @param r_pin mbed PinName that supports PWM output assigned to the red led
     *  @param g_pin mbed PinName that supports PWM output assigned to the green led
     *  @param b_pin mbed PinName that supports PWM output assigned to the blue led
     */
    RGB(PinName r_pin, PinName g_pin, PinName b_pin);
    ~RGB();

    /** Set the color by giving an instance of an Color object
     *  @param color Pointer to an instance of an Color object
     *  @ref Color
     */

	void setColor(Color& color);
    void setColor(Color&& color);

    /** Set the color by giving an integer in hexadecimal notation
     *  @param color Color in hexadecimal notation (hex triplet). 24-bit RGB color as used in web colors.
     *  @note Each color is made up of 8 bits, 0xRRGGBB
     */
    void setColor(int color);

    /** Get the current color of the RGB led
     *  @return instance of Color class containing the current set color
     *  @ref Color
     */
    Color getColor() const;


    /// Turn the led off
    void off();

    private:

    PwmOut* r_out;
    PwmOut* g_out;
    PwmOut* b_out;

    Color color;

    void setPwmColor(int value, PwmOut* output);

};

#endif

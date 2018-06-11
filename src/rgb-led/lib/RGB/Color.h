
#ifndef COLOR_H
#define COLOR_H


/** Color class.
 *  Used for easy management of colors. Colors can be set by giving individual values for the
 *  red, green and blue component, both in integer and floating point notation, or as a single
 *  integer formatted in hexadecimal format (hex triplet), used in web colors.
 */
class Color{

    int red;
    int green;
    int blue;

    void setColor(int red, int green, int blue);

    public:

    /// Enum with named colors for easy use
    enum Colors {RED = 0xFF0000, GREEN = 0x00FF00, BLUE = 0x0000FF, CYAN = 0x00FFFF, MAGENTA = 0xFF00FF, YELLOW = 0xFFFF00, WHITE = 0xFFFFFF, BLACK = 0x000000};

    /** Create Color instance, giving individual red, green and blue factors as integer values
     *  @param red Amount of red to be in the color. This value can be an integer between 0 (0% red) and 255 (100% red)
     *  @param green Amount of green to be in the color. This value can be an integer between 0 (0% green) and 255 (100% green)
     *  @param blue Amount of blue to be in the color. This value can be an integer between 0 (0% blue) and 255 (100% blue)
     */
    Color(int red, int green, int blue);

    /** Create Color instance, giving individual red, green and blue factors as floating point values
     *  @param red Amount of red to be in the color. This value can be a float between 0.0 (0% red) and 1.0 (100% red)
     *  @param green Amount of green to be in the color. This value can be a float between 0.0 (0% green) and 1.0 (100% green)
     *  @param blue Amount of blue to be in the color. This value can a float between 0.0 (0% blue) and 1.0 (100% blue)
     */
    Color(float red, float green, float blue);

	Color() { };

    /** Create Color instance, giving red, green and blue factors as a single integer value
     *  @param hexColor Color in hexadecimal notation (hex triplet). 24-bit RGB color as used in web colors.
     *  @note Each color is made up of 8 bits, 0xRRGGBB
     *
     *  Examples:
     *  @code
     *    0xFF0000 = Red
     *    0xFFFFE0 = Light yellow
     *    0xFA8072 = Salmon
     *  @endcode
     */
    Color(int hexColor);

    /** Get the color value as an integer in hexadecimal notation
     *  @return color as integer in hexadecimal notation (between 0x000000 and 0xFFFFFF)
     */
    int getHex();

    /** Get the red factor of the color
     *  @return red factor as integer between 0 (0% red) and 255 (100% red)
     */
    int getRed();

    /** Get the green factor of the color
     *  @return green factor as integer between 0 (0% green) and 255 (100% green)
     */
    int getGreen();

    /** Get the blue factor of the color
     *  @return blue factor as integer between 0 (0% blue) and 255 (100% blue)
     */
    int getBlue();
};

#endif

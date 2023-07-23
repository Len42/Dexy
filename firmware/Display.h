#pragma once

namespace Dexy {

/// @brief I2C interface to OLED display with text and drawing functions
/// @details Supports 128x32 and 128x64 Adafruit OLED displays, and probably others.
///
/// Text is displayed using a monospaced font. Text position is specified by the
/// x/y coordinates of the top left corner of the text bounding box.
///
/// Basic text and drawing functions are provided by the ssd1306 library
/// https://github.com/daschr/pico-ssd1306
namespace Display {

/// @brief true for Adafruit 1.3 in. 128x64, false for Adafruit 0.91 in. 128x32
constexpr bool isLargeDisplay = false;

/// @brief Screen width
/// @return Number of pixels
constexpr unsigned screenWidth();

/// @brief Screen height
/// @return Number of pixels
constexpr unsigned screenHeight();

/// @brief Width of one character
/// @return Number of pixels
/// @details Assumes a monospaced font.
constexpr unsigned charWidth();

/// @brief Height of one character
/// @return Number of pixels
/// @details Assumes a monospaced font.
constexpr unsigned charHeight();

/// @brief Initialization - must be called at startup
void init();

/// @brief Prepare to draw stuff on a blank screen
/// @note This must be called before drawing anything.
void startDrawing();

/// @brief Display what has been drawn since calling startDrawing()
/// @note This must be called after calling startDrawing() and a bunch of
/// drawing frunctions.
void endDrawing();

/// @brief Clear the screen
/// @note No need to call startDrawing() and endDrawing().
void clear();

#if false // UNUSED
/// @brief Draw a line
/// @param x1 
/// @param y1 
/// @param x2 
/// @param y2 
/// @note startDrawing() must be called before this.
void drawLine(int x1, int y1, int x2, int y2);

/// @brief Draw a rectangle
/// @param x 
/// @param y 
/// @param w 
/// @param h 
/// @note startDrawing() must be called before this.
void drawRect(unsigned x, unsigned y, unsigned w, unsigned h);
#endif

/// @brief Draw a circle
/// @param x X coordinate of the circle's centre
/// @param y Y coordinate of the circle's centre
/// @param r Radius of the circle
/// @note startDrawing() must be called before this.
void drawCircle(unsigned x, unsigned y, unsigned r);

/// @brief Display a character
/// @param ch The character to display
/// @param x X coordinate of the position
/// @param y Y coordinate of the position
/// @note startDrawing() must be called before this.
void drawText(char ch, unsigned x, unsigned y);

/// @brief Display a text string
/// @param str String pointer (does not have to be null terminated)
/// @param numChars Number of characters in the string (not including \0)
/// @param x X coordinate of the position
/// @param y Y coordinate of the position
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return Y coordinate of the bottom of the text (including wrapped lines,
/// if breakLines == true)
unsigned drawText(const char* str, unsigned numChars, unsigned x, unsigned y, bool breakLines);

/// @brief Display a text string
/// @tparam SIZE String length (including \0), deduced from the str argument
/// @param str Null-terminated string constant
/// @param x X coordinate of the position
/// @param y Y coordinate of the position
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return Y coordinate of the bottom of the text (including wrapped lines,
/// if breakLines == true)
template<size_t SIZE>
unsigned drawText(const char(&str)[SIZE], unsigned x, unsigned y, bool breakLines)
{
    return drawText(str, SIZE-1, x, y, breakLines);
}

/// @brief Display a text string from a std::array
/// @tparam SIZE String length, deduced from the str argument
/// @param str String to display
/// @param x X coordinate of the position
/// @param y Y coordinate of the position
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return Y coordinate of the bottom of the text (including wrapped lines,
/// if breakLines == true)
template<size_t SIZE>
unsigned drawText(const std::array<char,SIZE>& str, unsigned x, unsigned y, bool breakLines)
{
    return drawText(str.data(), SIZE, x, y, breakLines);
}

/// @brief Calculate the vertical space required to display the given string
/// @param str String pointer (does not have to be null terminated)
/// @param numChars Number of characters in the string (not including \0)
/// @param x X coordinate of the position (because where the text is displayed
/// can affect the amount of vertical space required)
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return The height of the given text
unsigned getTextHeight(const char* str, unsigned numChars, unsigned x, bool breakLines);

/// @brief Calculate the vertical space required to display the given string
/// @tparam SIZE String length (including \0), deduced from the str argument
/// @param str Null-terminated string constant
/// @param x X coordinate of the position (because where the text is displayed
/// can affect the amount of vertical space required)
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return The height of the given text
template<size_t SIZE>
unsigned getTextHeight(const char(&str)[SIZE], unsigned x, bool breakLines)
{
    return getTextHeight(str, SIZE-1, x, breakLines);
}

/// @brief Calculate the vertical space required to display the given string
/// @tparam SIZE String length, deduced from the str argument
/// @param str String to display
/// @param x X coordinate of the position (because where the text is displayed
/// can affect the amount of vertical space required)
/// @param breakLines If true, long text is broken at the right edge of the screen
/// and wrapped to the next line
/// @return The height of the given text
template<size_t SIZE>
unsigned getTextHeight(const std::array<char,SIZE>& str, unsigned x, bool breakLines)
{
    return getTextHeight(str.data(), SIZE, x, breakLines);
}

/// @brief Calculate the horizontal space required to display a text string with
/// the given number of characters
/// @param numChars Number of characters
/// @return The width of that many characters
/// @note Text font is monospaced, so only the number of characters matters.
unsigned getTextWidth(unsigned numChars);

/// @brief Calculate the vertical space required to display the given string
/// @tparam SIZE String length, deduced from the string argument
/// @return The width of the given text
template<size_t SIZE>
unsigned getTextWidth(const char(&)[SIZE])
{
    return getTextWidth(SIZE-1);
}

/// @brief Calculate the vertical space required to display the given string
/// @tparam SIZE String length, deduced from the string argument
/// @return The width of the given text
template<size_t SIZE>
unsigned getTextWidth(std::array<char,SIZE>&)
{
    return getTextWidth(SIZE);
}

/// @brief Display some stuff at startup
/// @details This should be called at some point during startup.
void showSplashScreen();

/// @brief Display a list of strings, one per line, with the selected string
/// highlighted
/// @details If the list is too long to fit on the screen, a portion of the list
/// including the selected item will be displayed.
/// @param itemSelected Index of the selected item
/// @param getItem Function or lambda to return the i'th string and its length
/// (not 0-terminated):
/// @code
/// auto getItem(int i) -> std::tuple<const char*, unsigned>;
/// @endcode
void showList(int itemSelected, auto getItem);

/// @brief Display a list of strings, one per line
/// @details If the list is too long to fit on the screen, the first portion of
/// the listwill be displayed.
/// @param getItem Function or lambda to return the i'th string and its length
/// (not 0-terminated):
/// @code
/// auto getItem(int i) -> std::tuple<const char*, unsigned>;
/// @endcode
void showList(auto getItem);

} } // namespace Display

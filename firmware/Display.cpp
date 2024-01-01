extern "C" {
#include "ssd1306.h"
#include "font_Dina_r400_10.h"
}

namespace Dexy { namespace Display {

/// @brief Display interface object from the ssd1306 library
static ssd1306_t display;

/// @brief Font scaling is fixed at 1:1
static constexpr unsigned fontScale = 1;

/// @brief Text is displayed using a monospaced font
static constexpr auto& fontUI = fontDina_r400_10;

constexpr unsigned screenWidth() { return 128; }

constexpr unsigned screenHeight() { return isLargeDisplay ? 64: 32; }

/// @brief Character width, based on the font, including the space in between
static constexpr unsigned charWidthFont = fontUI[1] + fontUI[2];

/// @brief Character height, based on the font, including the space in between
/// https://youtu.be/IWafhS_9L9I?t=1035
static constexpr unsigned charHeightFont = fontUI[0];

constexpr unsigned charWidth() { return charWidthFont * fontScale; }

constexpr unsigned charHeight() { return charHeightFont * fontScale; }

/// @brief I2C address of the display device
/// @details Different for large and small display modules
constexpr uint8_t i2cAddress = isLargeDisplay ? 0x3D : 0x3C;

IN_FLASH("Display")
void init()
{
    i2c_init(Gpio::i2cInstance, 400000);
    gpio_set_function(Gpio::pinI2cSda, GPIO_FUNC_I2C);
    gpio_pull_up(Gpio::pinI2cSda);
    gpio_set_function(Gpio::pinI2cScl, GPIO_FUNC_I2C);
    gpio_pull_up(Gpio::pinI2cScl);

    display.external_vcc = false;
    ssd1306_init(&display, screenWidth(), screenHeight(), i2cAddress, Gpio::i2cInstance);
}

IN_FLASH("Display")
void startDrawing()
{
    ssd1306_clear(&display);
}

IN_FLASH("Display")
void endDrawing()
{
    ssd1306_show(&display);
}

IN_FLASH("Display")
void clear()
{
    startDrawing();
    endDrawing();
}

#if false // UNUSED
IN_FLASH("Display")
void drawLine(int x1, int y1, int x2, int y2)
{
    ssd1306_draw_line(&display, x1, y1, x2, y2);
}
IN_FLASH("Display")
void drawRect(unsigned x, unsigned y, unsigned w, unsigned h)
{
    ssd1306_draw_square(&display, x, y, w, h);
}
#endif

IN_FLASH("Display")
void drawCircle(unsigned x, unsigned y, unsigned r)
{
    ssd1306_draw_empty_circle(&display, x, y, r);
}

IN_FLASH("Display")
void drawText(char ch, unsigned x, unsigned y)
{
    ssd1306_draw_char_with_font(&display, x, y, fontScale, fontUI, ch);
}

/// @brief Helper function for drawText() and getTextHeight()
/// @param str 
/// @param x 
/// @param y 
/// @param breakLines 
/// @param doDraw If true, draw the text; otherwise return text height without drawing
/// @return Text height
IN_FLASH("Display")
static unsigned drawText(std::string_view str, unsigned x, unsigned y, bool breakLines, bool doDraw)
{
    if (x > screenWidth() - charWidth())
        return 0; // this avoids an infinite loop with bad args
    unsigned xStart = x;
    for (char ch : str) {
        if (x > screenWidth() - charWidth()) {
            x = xStart;
            y += charHeight();
            if (!breakLines || y >= screenHeight())
                return y;
        }
        if (doDraw) {
            ssd1306_draw_char_with_font(&display, x, y, fontScale, fontUI, ch);
        }
        x += charWidth();
    }
    return y + charHeight();
}

IN_FLASH("Display")
unsigned drawText(std::string_view str, unsigned x, unsigned y, bool breakLines)
{
    return drawText(str, x, y, breakLines, /*doDraw*/true);
}

IN_FLASH("Display")
unsigned getTextHeight(std::string_view str, unsigned x, bool breakLines)
{
    return drawText(str, x, 0, breakLines, /*doDraw*/false);
}

IN_FLASH("Display")
unsigned getTextWidth(unsigned numChars)
{
    return numChars * charWidth();
}

IN_FLASH("Display")
void showSplashScreen()
{
    startDrawing();
    static constexpr char strTitle[] = "Dexy ";
    unsigned widthTitle = getTextWidth(strTitle);
    drawText(strTitle, 0, 0, false);
    if constexpr (VersionInfo::isDevBuild) {
        drawText(VersionInfo::nameDev, widthTitle, 0, true);
    } else {
        drawText(VersionInfo::name, widthTitle, 0, true);
    }
    endDrawing();
}

IN_FLASH("Display")
void showList(int itemSelected, auto getItem)
{
    startDrawing();
    int maxLines = (screenHeight() + 1) / charHeight();
    int lineSelected = (maxLines - 1) / 2;
    // itemSelected < 0 means no item selected
    bool fSelection = (itemSelected >= 0);
    int itemTop = (fSelection ? itemSelected : lineSelected) - lineSelected;
    unsigned xLeft = fSelection ? charWidth() : 0;
    int i;
    unsigned y;
    for (i = itemTop, y = 0U; y < screenHeight(); ++i, y += charHeight()) {
        if (i >= 0) {
            auto str = getItem(i);
            if (str.empty())
                break;
            drawText(str, xLeft, y, false);
        }
    }
    if (fSelection) {
        drawText('>', 0, lineSelected * charHeight());
    }
    endDrawing();
}

IN_FLASH("Display")
void showList(auto getItem)
{
    showList(-1, getItem);
}

} } // namespace Display

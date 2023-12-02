#ifndef NATIVE

#include <cstring>
#include <qrcode.h>

#include "u8g_display.h"
#include "formatters.h"
#include "data/bitmaps.h"
#include "constants.h"
#include "data/localization.h"

#define Y_PADDING 4

#define FONT_SMALL u8g2_font_profont12_tf
#define FONT_SMALL_MEDIUM u8g2_font_profont15_tf
#define FONT_MEDIUM u8g2_font_profont17_tf
#define FONT_LARGE u8g2_font_logisoso20_tf

static const char *TAG = "U8GDISP";

U8GDisplay::U8GDisplay(const uint8_t pin_sda, const uint8_t pin_scl, const u8g2_cb_t *rotation)
    : u8g(rotation, U8X8_PIN_NONE, pin_scl, pin_sda)
{
}

void U8GDisplay::begin()
{
    u8g.setBusClock(1000000);
    u8g.begin();
}

void U8GDisplay::update()
{
    u8g.sendBuffer();
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_VERBOSE
    frameCounter++;
    // every 120 frames log the fps
    if (frameCounter >= 120)
    {
        unsigned long time = millis();

        uint8_t fps = (1000.0 / (time - lastFpsLogTime)) * frameCounter;
        ESP_LOGD(TAG, "FPS: %d", fps);

        lastFpsLogTime = time;
        frameCounter = 0;
    }
#endif
}

void U8GDisplay::clear()
{
    u8g.clearBuffer();
}

void U8GDisplay::display(float weight, unsigned long time)
{
    char *weightText = formatWeight(weight);
    char *timeText = formatTime(time);
    u8g.clearBuffer();
    u8g.setFont(u8g2_font_logisoso30_tf);
    u8g.drawStr(0, 30, weightText);
    u8g.setFont(u8g2_font_logisoso22_tf);
    u8g.drawStr(0, 64, timeText);
}

void U8GDisplay::promptText(const char *prompt, const char *text)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);
    u8g.drawStr(0, 10, prompt);
    u8g.drawStr(0, 20, text);
}

void U8GDisplay::drawHCenterText(const char *text, uint8_t y)
{
    u8g.drawUTF8(u8g.getDisplayWidth() / 2.0 - u8g.getUTF8Width(text) / 2.0, y, text);
}

void U8GDisplay::drawCenterText(const char *text)
{
    drawHCenterText(text, u8g.getDisplayHeight() / 2.0 + u8g.getFontAscent() / 2.0);
}

void U8GDisplay::centerText(const char *text, const uint8_t size)
{
    u8g.clearBuffer();

    int mid = 0;
    switch (size)
    {
    case 13:
        u8g.setFont(u8g_font_7x13);
        mid = 32 + 6;
        break;
    case 16:
        u8g.setFont(u8g2_font_logisoso16_tf);
        mid = 32 + 8;
        break;
    case 30:
        u8g.setFont(u8g2_font_logisoso30_tf);
        mid = 32 + 15;
        break;
    default:
        u8g.setFont(u8g_font_6x10);
        mid = 32 + 5;
        break;
    }

    drawHCenterText(text, mid);
}

int U8GDisplay::drawTitleLine(const char *title)
{
    u8g.setFont(FONT_SMALL);

    int ascent = u8g.getAscent();
    int descent = u8g.getDescent();
    int width = u8g.getDisplayWidth();
    int yy = ascent - descent;

    // draw title line
    u8g.drawUTF8(width / 2.0 - u8g.getUTF8Width(title) / 2.0, yy, title);
    yy += Y_PADDING;
    u8g.drawHLine(0, yy, width);
    return yy;
}

void U8GDisplay::switcher(const char* title, const uint8_t index, const uint8_t count, const char *options[])
{
    u8g.clearBuffer();

    int yy = drawTitleLine(title);
    yy += 2;

    u8g.setFont(FONT_SMALL);
    u8g.setFontPosBaseline();
    int ascent = u8g.getAscent();
    int descent = u8g.getDescent();
    int height = u8g.getDisplayHeight();
    int width = u8g.getDisplayWidth();

    int optionHeight = ascent + 2;
    int visibleOptionsCount = (height - yy) / optionHeight;
    if (visibleOptionsCount > count)
    {
        visibleOptionsCount = count;
    }

    int firstVisibleOptionIndex = index - visibleOptionsCount / 2;
    if (firstVisibleOptionIndex < 0)
    {
        firstVisibleOptionIndex = 0;
    }
    else if (firstVisibleOptionIndex + visibleOptionsCount > count)
    {
        // if last displayed item would be out of bounds, start displaying at the last possible item
        firstVisibleOptionIndex = count - visibleOptionsCount;
    }

    // yy in for loop is the top of the current option
    for (int i = 0; i < visibleOptionsCount; i++)
    {
        int optionIndex = firstVisibleOptionIndex + i;

        // when option is selected, draw inverted
        if (optionIndex == index)
        {
            u8g.drawBox(0, yy, width, optionHeight);
            u8g.setDrawColor(0);
        }

        u8g.drawUTF8(2, yy + ascent + 1, options[optionIndex]);
        u8g.setDrawColor(1);
        yy += optionHeight;
    }
};

#define QR_CODE_VERSION 2
int U8GDisplay::drawQRCode(const char *bytes, uint8_t posX, uint8_t posY)
{
    static QRCode qrcode;
    static uint8_t *qrCodeBytes = new uint8_t[qrcode_getBufferSize(QR_CODE_VERSION)];

    qrcode_initText(&qrcode, qrCodeBytes, QR_CODE_VERSION, ECC_QUARTILE, bytes);

    // draw box with 2 px overlap
    const uint8_t border = 2;
    u8g.drawBox(posX, posY, 2 * qrcode.size + 2 * border, 2 * qrcode.size + 2 * border);
    u8g.setDrawColor(0);
    for (uint8_t y = 0; y < qrcode.size; y++)
    {
        for (uint8_t x = 0; x < qrcode.size; x++)
        {
            if (qrcode_getModule(&qrcode, x, y))
            {
                u8g.drawBox(2 * x + posX + border, 2 * y + posY + border, 2, 2);
            }
        }
    }
    u8g.setDrawColor(1);

    return 2 * qrcode.size + 2 * border;
}

void U8GDisplay::recipeSummary(const char *name, const char *description, const char *url)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);

    int ascent = u8g.getAscent();
    int descent = u8g.getDescent();

    int height = u8g.getDisplayHeight();
    int width = u8g.getDisplayWidth();

    if (url == nullptr)
    {
        int yy = drawTitleLine(name);
        drawTextAutoWrap(description, yy + 2, 0, width);
    }
    else
    {
        // u8g.drawBox(0, 0, u8g.getWidth(), u8g.getHeight());
        // u8g.setDrawColor(0);
        drawTextAutoWrap(description, 0, 1, width - 54 - 3);
        // u8g.setDrawColor(1);

        // qr code size 54
        drawQRCode(url, width - 54, (height - 54) / 2.0);
    }
}

bool U8GDisplay::shouldBlinkedBeVisible()
{
    // return false each 4th second
    return millis() % 1000 > 200;
}

void U8GDisplay::recipeConfigCoffeeWeight(const char *header, unsigned int weightMg, unsigned int waterWeightMl)
{
    u8g.clearBuffer();
    int yy = drawTitleLine(header);
    yy += Y_PADDING;

    int remainingHeight = u8g.getDisplayHeight() - yy;
    static char buffer[16];
    static const int X_OFFSET = 10;
    // draw coffee string
    u8g.setFontPosCenter();
    u8g.setFont(FONT_SMALL_MEDIUM);
    static const char *coffee = DISPLAY_CONFIG_WEIGHT_COFFEE;
    u8g.drawStr(u8g.getDisplayWidth() / 2.0 - u8g.getStrWidth(coffee) - X_OFFSET, yy + (remainingHeight / 4.0), coffee);
    if (shouldBlinkedBeVisible())
    {
        sprintf(buffer, "%.1fg", weightMg / 1000.0);
        u8g.setFont(FONT_MEDIUM);
        u8g.drawStr(u8g.getWidth() / 2.0, yy + (remainingHeight / 4.0), buffer);
    }

    // draw water string
    u8g.setFont(FONT_SMALL_MEDIUM);
    static const char *water = DISPLAY_CONFIG_WEIGHT_WATER;
    u8g.drawStr(u8g.getDisplayWidth() / 2.0 - u8g.getStrWidth(water) - X_OFFSET, yy + (remainingHeight / 4.0) * 3, water);
    sprintf(buffer, "%dml", waterWeightMl);
    u8g.setFont(FONT_MEDIUM);
    u8g.drawStr(u8g.getWidth() / 2.0, yy + (remainingHeight / 4.0) * 3, buffer);

    u8g.setFontPosBaseline();
}

void U8GDisplay::recipeConfigRatio(const char *header, uint32_t coffee, uint32_t water)
{
    u8g.clearBuffer();
    int yy = drawTitleLine(header);
    yy += 2 * Y_PADDING;

    u8g.setFont(FONT_MEDIUM);
    yy += u8g.getAscent();
    drawHCenterText(DISPLAY_CONFIG_RATIO, yy);
    yy += Y_PADDING;

    u8g.setFont(FONT_LARGE);
    yy += (u8g.getDisplayHeight() - yy) / 2.0 + u8g.getAscent() / 2.0;

    // draw colon in center
    u8g.drawStr(u8g.getDisplayWidth() / 2.0 - u8g.getStrWidth(":") / 2.0, yy, ":");

    static char buffer[16];

    // draw left side
    sprintf(buffer, "%.1f", coffee / 10.0);
    u8g.drawStr(u8g.getDisplayWidth() / 4.0 - u8g.getStrWidth(buffer) / 2.0, yy, buffer);
    // draw right side, only if blink should show
    if (shouldBlinkedBeVisible())
    {
        sprintf(buffer, "%.1f", water / 10.0);
        u8g.drawStr(3 * u8g.getDisplayWidth() / 4.0 - u8g.getStrWidth(buffer) / 2.0, yy, buffer);
    }
}

int U8GDisplay::drawSelectedBar(uint8_t index, uint8_t size)
{
    int width = u8g.getDisplayWidth();
    int ascent = u8g.getAscent();

    // display header shwoing number of pours as rectangles with current pur highlighted by a filled rectangle
    static const int PROGRESS_HEIGHT = 4;
    int boxWidth = width / size;
    u8g.drawFrame(0, 0, width, PROGRESS_HEIGHT);
    for (int i = 1; i < size; i++)
    {
        u8g.drawVLine(i * boxWidth, 0, PROGRESS_HEIGHT);
    }
    u8g.drawBox(index * boxWidth, 0, boxWidth, PROGRESS_HEIGHT);

    return PROGRESS_HEIGHT;
}

void U8GDisplay::recipeInsertCoffee(int32_t weightMg, uint32_t requiredWeightMg)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_7x13);

    drawHCenterText(DISPLAY_INSERT_COFFEE, u8g.getAscent() + 5);

    u8g.setFont(u8g_font_9x18);
    static char buffer[16];
    sprintf(buffer, "%.2fg/%.1fg", weightMg / 1000.0, requiredWeightMg / 1000.0);
    drawCenterText(buffer);
}

void U8GDisplay::recipePour(const char *text, int32_t weightToPourMg, uint64_t timeToFinishMs, bool isPause, uint8_t pourIndex, uint8_t pours)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);

    int width = u8g.getDisplayWidth();
    int ascent = u8g.getAscent();

    // display header shwoing number of pours as rectangles with current pur highlighted by a filled rectangle
    int yy = drawSelectedBar(pourIndex, pours);

    // draw info text
    yy += 2;
    drawTextAutoWrap(text, yy, 0, width);

    // draw bottom: time and weight
    u8g.setFont(u8g_font_7x13);
    yy = u8g.getDisplayHeight() - (u8g.getAscent() - u8g.getDescent()) - 6;
    u8g.drawHLine(0, yy, width);
    yy += 3;

    // get center of remaining y space
    int center = yy + (u8g.getDisplayHeight() - yy) / 2.0;

    // TODO add lines showing progress of weight and time
    const static int Y_SPACING = 5;
    static char buffer[16];

    sprintf(buffer, "%.2fg", -1 * weightToPourMg / 1000.0);

    const static int TEXT_X_PADDING = 3;
    u8g.drawStr(TEXT_X_PADDING, center + ascent / 2.0, buffer);

    if (isPause)
    {
        sprintf(buffer, "TP-%02d:%02d", (int)(timeToFinishMs / 1000 / 60), (int)(timeToFinishMs / 1000 % 60));
    }
    else
    {
        sprintf(buffer, "T-%02d:%02d", (int)(timeToFinishMs / 1000 / 60), (int)(timeToFinishMs / 1000 % 60));
    }
    int textWidth = u8g.getStrWidth(buffer);
    u8g.drawStr(width - textWidth - TEXT_X_PADDING, center + ascent / 2.0, buffer);
}

void U8GDisplay::text(const char *text)
{
    u8g.clearBuffer();
    u8g.setFont(u8g_font_6x10);

    // split string at newline
    char *textCopy = strdup(text);
    char *pointer = strtok(textCopy, "\n");
    int line = 10;

    while (pointer != NULL)
    {
        u8g.drawStr(0, line, pointer);
        line += 10;
        pointer = strtok(NULL, "\n");
    }
    delete textCopy;
}

void U8GDisplay::drawTextAutoWrap(const char *text, int yTop, int xLeft, int maxWidth)
{
    u8g.setFont(u8g_font_6x10);
    int ascent = u8g.getAscent();
    int descent = u8g.getDescent();
    int spaceWidth = u8g.getStrWidth(" ");

    char *textCopy = strdup(text);
    char *pointer = strtok(textCopy, " ");

    int line = (ascent - descent) + yTop;
    int x = xLeft;
    while (pointer != NULL)
    {
        if (x + u8g.getUTF8Width(pointer) > maxWidth)
        {
            x = xLeft;
            line += (ascent - descent);
        }
        u8g.drawUTF8(x, line, pointer);
        x += u8g.getUTF8Width(pointer) + spaceWidth;
        pointer = strtok(NULL, " ");
    }
    delete textCopy;
}

void U8GDisplay::modeSwitcher(const char *current, const uint8_t index, const uint8_t count, float batV, float batPercentage, bool batCharging)
{
    u8g.clearBuffer();

    drawSelectedBar(index, count);

    u8g.setFont(u8g_font_10x20);
    drawCenterText(current);

    // battery state
    u8g.setFont(u8g2_font_battery19_tn);
    uint16_t batGlyph = 0x0030;
    if (batCharging)
    {
        batGlyph += 6;
    }
    else
    {
        batGlyph += roundf(batPercentage / 20);
    }
    u8g.setFontDirection(1);
    static const int PADDING = 5;
    u8g.drawGlyph(PADDING, u8g.getDisplayHeight() - 8 - PADDING, batGlyph);
    u8g.setFontDirection(0);

    if (!batCharging)
    {
        u8g.setFont(u8g_font_6x10);
        static char buffer[16];
        sprintf(buffer, "%.2fV", batV);
        int textWidth = u8g.getUTF8Width(buffer);
        u8g.drawUTF8(u8g.getDisplayWidth() - textWidth - PADDING, u8g.getDisplayHeight() - PADDING, buffer);
    }
}

void U8GDisplay::drawOpener()
{
    u8g.clearBuffer();

    u8g.drawXBM(0, 0, chemex_width, chemex_height, chemex_bits);

    u8g.setFont(u8g_font_10x20);
    const static char *textLine1 = "Coffee";
    const static char *textLine2 = "Scale";
    int ascent = u8g.getAscent();
    int width = u8g.getDisplayWidth();
    int height = u8g.getDisplayHeight();

    int remainingCenter = chemex_width + (width - chemex_width) / 2.0;

    int textWidth = u8g.getStrWidth(textLine1);
    int yy = 6 + ascent;
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLine1);
    yy += ascent + 4;
    textWidth = u8g.getStrWidth(textLine2);
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLine2);

    u8g.setFont(u8g_font_7x13);
    ascent = u8g.getAscent();
    const static char *textLineUrl = "orlopau.dev";
    const static char *textLineVersion = FIRMWARE_VERSION;
    textWidth = u8g.getStrWidth(FIRMWARE_VERSION);

    yy += ascent + 5;
    textWidth = u8g.getStrWidth(textLineUrl);
    u8g.drawStr(remainingCenter - textWidth / 2.0, yy, textLineUrl);

    u8g.setFont(u8g_font_6x12);
    ascent = u8g.getAscent();
    textWidth = u8g.getStrWidth(textLineVersion);
    u8g.drawStr(remainingCenter - textWidth / 2.0, height - 2, textLineVersion);
}

void U8GDisplay::espressoShot(uint32_t currentTimeMs, uint32_t timeToFinishMs, int32_t currentWeightMg, uint32_t targetWeightMg,
                              bool waiting)
{
    u8g.clearBuffer();
    int width = u8g.getDisplayWidth();
    int height = u8g.getDisplayHeight();

    u8g.setFont(u8g2_font_logisoso18_tf);
    int ascent = u8g.getAscent();

    int barHeight = 5;
    int barWidth = width;

    int yy = 4;
    int xx = 2;

    // draw time to finish
    float timeToFinishS = timeToFinishMs / 1000.0;
    float currentTimeS = currentTimeMs / 1000.0;
    static char buffer[16];
    if (waiting)
    {
        sprintf(buffer, "%.1fs", currentTimeS);
    }
    else
    {
        sprintf(buffer, "%.1fs|%.1fs", -timeToFinishS, currentTimeS);
    }
    int textWidth = u8g.getUTF8Width(buffer);
    u8g.drawUTF8(width / 2.0 - textWidth / 2.0, yy + ascent, buffer);

    // change font
    u8g.setFont(u8g2_font_logisoso16_tf);
    ascent = u8g.getAscent();
    yy = 2 + ascent;

    // draw current weight in g
    float currentWeightG = currentWeightMg / 1000.0;
    float targetWeightG = targetWeightMg / 1000.0;
    sprintf(buffer, "%.1fg/%.1fg", currentWeightG, targetWeightG);
    textWidth = u8g.getUTF8Width(buffer);
    u8g.drawUTF8(width / 2.0 - textWidth / 2.0, height - barHeight - 8, buffer);

    // draw full width weight progress bar on bottom
    int barY = height - barHeight;
    int barX = 0;
    int barProgress = (currentWeightG / targetWeightG) * barWidth;
    u8g.drawFrame(barX, barY, barWidth, barHeight);
    u8g.drawBox(barX, barY, barProgress, barHeight);
}

#endif
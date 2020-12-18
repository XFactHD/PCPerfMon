#include "display.hpp"

#ifdef __IMXRT1062__
uint8_t data[16] = { 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
ST7789 display = ST7789(TFT_RST, TFT_RD, TFT_WR, TFT_CS, TFT_DC, data, TFT_BL);
#elif defined(__SAMD21G18A__) || defined(__SAMD51__)
ST7789 display = ST7789(TFT_RST, TFT_RD, TFT_WR, TFT_CS, TFT_DC, &PORT->Group[PORTA], 6, TFT_BL);
#endif

void initDisplay() {
    display.begin(false);
    display.setRotation(3);
    display.fillScreen(TFT_WHITE);
    display.setTextColor(TFT_BLACK, TFT_WHITE);

    drawBackground();
}

void drawBackground() {
    display.fillRect(0, 0, 320, 36, TFT_CYAN);
    display.drawRect(0, 0, 320, 36, TFT_BLACK);
    display.drawRect(  0, 0, 80, 240, TFT_BLACK);
    display.drawRect( 80, 0, 80, 240, TFT_BLACK);
    display.drawRect(160, 0, 80, 240, TFT_BLACK);
    display.drawRect(240, 0, 80, 240, TFT_BLACK);

    display.setTextSize(2);
    display.setTextColor(TFT_BLACK, TFT_CYAN);

    display.setCursor(22, 10);
    display.print("CPU");

    display.setCursor(102, 10);
    display.print("RAM");

    display.setCursor(182, 10);
    display.print("NET");

    display.setCursor(262, 10);
    display.print("GPU");

    display.setTextSize(1);
    display.setTextColor(TFT_BLACK, TFT_WHITE);
}

void enableDisplay() {
    display.setDisplayOn(true);
}

void disableDisplay() {
    display.setDisplayOn(false);
}

ST7789* getDisplay() {
    return &display;
}

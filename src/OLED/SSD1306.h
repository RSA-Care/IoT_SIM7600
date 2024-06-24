#if !defined(SSD1306_h)
#define SSD1306_h

#include <Arduino.h>

#define OLED_I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin

void oledBegin();
void print(String message);
void println(String message);
void clearScreen();

#endif // SSD1306_h
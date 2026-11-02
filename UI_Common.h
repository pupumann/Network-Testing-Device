#ifndef UI_COMMON_H
#define UI_COMMON_H

#include <TFT_eSPI.h>

extern TFT_eSPI tft;

#define COLOR_BG        0x0000
#define COLOR_HEADER    0x18E3
#define COLOR_TEXT      0xFFFF
#define COLOR_DIM       0x7BEF
#define COLOR_CYAN      0x07FF
#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_YELLOW    0xFFE0
#define COLOR_BTN       0x2124
#define COLOR_SHADOW    0x1082

extern int currentState;
extern bool touchLocked; 

// Fungsi Helper Horizontal
void drawTerminalHeader(const char* title, uint16_t color);
void drawTechButton(int x, int y, int w, int h, const char* text, uint16_t color);
void drawBackButton();

#endif
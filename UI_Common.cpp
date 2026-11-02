#include "UI_Common.h"

int currentState = 0; 
bool touchLocked = false;

void drawTerminalHeader(const char* title, uint16_t color) {
  tft.fillRect(0, 0, 320, 35, COLOR_BG);
  tft.drawFastHLine(0, 35, 320, color);
  
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.print("> ");
  tft.print(title);

  tft.drawRect(280, 8, 30, 18, color);
  tft.setTextSize(1);
  tft.setCursor(288, 13);
  tft.print("SYS");
}

void drawTechButton(int x, int y, int w, int h, const char* text, uint16_t color) {
  tft.fillRect(x, y, w, h, COLOR_BTN);
  tft.drawRect(x, y, w, h, color);
  tft.drawRect(x+2, y+2, w-4, h-4, COLOR_SHADOW);
  
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, x + w/2, y + h/2);
  tft.setTextDatum(TL_DATUM);
}

void drawBackButton() {
  tft.fillRect(10, 215, 300, 20, COLOR_BG);
  tft.drawRect(10, 215, 300, 20, COLOR_RED);
  tft.setTextColor(COLOR_RED);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("[ ESC ] KEMBALI", 160, 225);
  tft.setTextDatum(TL_DATUM);
}